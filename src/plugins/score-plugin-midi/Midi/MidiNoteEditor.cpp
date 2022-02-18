#include "MidiNoteEditor.hpp"

#include <Midi/Commands/AddNote.hpp>
#include <Midi/Commands/RemoveNotes.hpp>
#include <Midi/MidiNote.hpp>
#include <Midi/MidiPresenter.hpp>
#include <Midi/MidiProcess.hpp>
#include <Midi/MidiView.hpp>

#include <score/command/Dispatchers/CommandDispatcher.hpp>
#include <score/document/DocumentContext.hpp>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <Scenario/Document/ScenarioDocument/ProcessFocusManager.hpp>

namespace Midi
{
bool NoteEditor::copy(
    JSONReader& r,
    const Selection& s,
    const score::DocumentContext& ctx)
{
  if (!s.empty())
  {
    std::vector<Midi::NoteData> noteDataList;
    for (auto item : s)
    {
      if (auto model = qobject_cast<const Midi::Note*>(item.data()))
      {
        noteDataList.push_back(model->noteData());
      }
    }
    if (!noteDataList.empty())
    {
      r.stream.StartObject();
      r.obj["Notes"] = noteDataList;
      r.stream.EndObject();
      return true;
    }
  }
  return false;
}

bool NoteEditor::paste(
    QPoint pos,
    const QMimeData& mime,
    const score::DocumentContext& ctx)
{
  auto focus = Process::ProcessFocusManager::get(ctx);
  if (!focus)
    return false;

  auto pres = qobject_cast<Midi::Presenter*>(focus->focusedPresenter());
  if (!pres)
    return false;

  auto& mm = static_cast<const Midi::ProcessModel&>(pres->model());
  // Get the QGraphicsView
  auto views = pres->view().scene()->views();
  if (views.empty())
    return false;

  auto view = views.front();

  // Find where to paste in the scenario
  auto view_pt = view->mapFromGlobal(pos);
  auto scene_pt = view->mapToScene(view_pt);
  auto& mv = pres->view();
  auto mv_pt = mv.mapFromScene(scene_pt);

  // TODO this is a bit lazy.. find a better positoning algorithm7
  //Position in the case the pasting is done through ui
  //  if (!mv.contains(mv_pt))
  //    mv_pt = mv.mapToScene(mv.boundingRect().center());

  // Read the copy json. TODO: give it a better mime type
  //  auto origin = pres->toScenarioPoint(sv_pt);
  auto obj = readJson(mime.data("text/plain"));
  JSONWriter w(obj);
  std::vector<NoteData> data;
  if (auto notes = w.obj.tryGet("Notes"))
    data = notes->to<std::vector<NoteData>>();
  else
    return false;
  if (data.empty())
    return false;

  // TODO check json validity
  // Submit the paste command
  //  auto cmd = new Midi::AddNote(mm,notesData)
  //  CommandDispatcher<>{ctx.commandStack}.submit(cmd);
  //  return true;

  return false;
}

bool NoteEditor::remove(const Selection& s, const score::DocumentContext& ctx)
{
  if (!s.empty())
  {
    std::vector<Id<Note>> noteIdList;
    for (auto item : s)
    {
      if (auto model = qobject_cast<const Midi::Note*>(item.data()))
      {
        if (auto parent
            = qobject_cast<const Midi::ProcessModel*>(model->parent()))
        {
          noteIdList.push_back(model->id());
        }
      }
    }
    if (!noteIdList.empty())
    {
      auto parent = qobject_cast<const Midi::ProcessModel*>(
          s.begin()->data()->parent());
      CommandDispatcher<>{ctx.commandStack}.submit<Midi::RemoveNotes>(
          *parent, noteIdList);
      return true;
    }
  }
  return false;
}
}
