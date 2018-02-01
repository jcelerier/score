#pragma once
#include <score/model/ComponentFactory.hpp>
#include <score/model/ComponentHierarchy.hpp>
#include <score/plugins/customfactory/ModelFactory.hpp>
#include <Media/Effect/EffectProcessModel.hpp>
#include <Engine/Executor/ProcessComponent.hpp>
#include <ossia/dataflow/node_process.hpp>
#include <ossia/dataflow/fx_node.hpp>
namespace Engine
{
namespace Execution
{
struct effect_chain_process final :
    public ossia::time_process
{
    effect_chain_process()
    {
      m_lastDate = ossia::Zero;
    }
    void
    state(ossia::time_value parent_date, double relative_position, ossia::time_value tick_offset, double speed) override
    {
      const ossia::token_request tk{parent_date, relative_position, tick_offset, speed};
      for(auto& node : nodes)
      {
        node->requested_tokens.push_back(tk);
      }
      m_lastDate = parent_date;
    }

    void add_node(std::shared_ptr<ossia::graph_node> n)
    {
      n->set_prev_date(this->m_lastDate);
      nodes.push_back(std::move(n));
    }
    void stop() override
    {
      for(auto& node : nodes)
      {
        node->all_notes_off();
      }
    }
    std::vector<std::shared_ptr<ossia::graph_node>> nodes;
};

class SCORE_PLUGIN_ENGINE_EXPORT EffectProcessComponentBase
    : public ::Engine::Execution::
    ProcessComponent_T<Media::Effect::ProcessModel, effect_chain_process>
{
  COMPONENT_METADATA("d638adb3-64da-4b6e-b84d-7c32684fa79d")
public:
    using parent_t = Engine::Execution::Component;
    using model_t = Process::ProcessModel;
    using component_t = ProcessComponent;
    using component_factory_list_t = ProcessComponentFactoryList;
  EffectProcessComponentBase(
      Media::Effect::ProcessModel& element,
      const ::Engine::Execution::Context& ctx,
      const Id<score::Component>& id,
      QObject* parent);

  void recompute();

  ~EffectProcessComponentBase() override;


  ProcessComponent* make(
          const Id<score::Component> & id,
          ProcessComponentFactory& factory,
          Process::ProcessModel &process);
  void added(ProcessComponent& e);

  std::function<void()> removing(
      const Process::ProcessModel& e,
      ProcessComponent& c);
  template <typename Component_T, typename Element, typename Fun>
  void removed(const Element& elt, const Component_T& comp, Fun f)
  {
    if(f)
      f();
  }

  template <typename Models>
  auto& models() const
  {
    static_assert(
        std::is_same<Models, Process::ProcessModel>::value,
        "Effect component must be passed Process::EffectModel as child.");

    return process().effects();
  }

private:
  struct RegisteredEffect
  {
      std::shared_ptr<ProcessComponent> comp;

      Process::Inlets registeredInlets;
      Process::Outlets registeredOutlets;

      const auto& node() const { return comp->node; }
      operator bool() const { return bool(comp); }
  };
  std::vector<std::pair<Id<Process::ProcessModel>, RegisteredEffect>> m_fxes;


  void unreg(const RegisteredEffect& fx);
  void reg(const RegisteredEffect& fx);
};


class SCORE_PLUGIN_ENGINE_EXPORT EffectProcessComponent final :
        public score::PolymorphicComponentHierarchy<EffectProcessComponentBase, false>
{
    public:
  EffectProcessComponent(
      Media::Effect::ProcessModel& element,
      const ::Engine::Execution::Context& ctx,
      const Id<score::Component>& id,
      QObject* parent):
    score::PolymorphicComponentHierarchy<EffectProcessComponentBase, false>{
      score::lazy_init_t{}, element, ctx, id, parent}
  {
    if(!element.badChaining())
      init_hierarchy();

    connect(&element, &Media::Effect::ProcessModel::badChainingChanged, this, [&] (bool b) {
      if(b)
      {
        clear();
      }
      else
      {
        init_hierarchy();
      }
    });
  }

  void cleanup() override;

  EffectProcessComponent(const EffectProcessComponent&) = delete;
  EffectProcessComponent(EffectProcessComponent&&) = delete;
  EffectProcessComponent& operator=(const EffectProcessComponent&) = delete;
  EffectProcessComponent& operator=(EffectProcessComponent&&) = delete;
  ~EffectProcessComponent();
};

using EffectProcessComponentFactory
= ::Engine::Execution::ProcessComponentFactory_T<EffectProcessComponent>;
}
}


SCORE_CONCRETE_COMPONENT_FACTORY(
    Engine::Execution::ProcessComponentFactory,
    Engine::Execution::EffectComponentFactory)
