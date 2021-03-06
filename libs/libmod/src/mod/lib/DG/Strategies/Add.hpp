#ifndef MOD_LIB_DG_STRATEGIES_ADD_H
#define MOD_LIB_DG_STRATEGIES_ADD_H

#include <mod/lib/DG/Strategies/Strategy.hpp>

namespace mod {
template<typename Sig>
class Function;
namespace lib {
namespace DG {
namespace Strategies {

struct Add : Strategy {
	Add(const std::vector<std::shared_ptr<graph::Graph> > graphs, bool onlyUniverse, IsomorphismPolicy graphPolicy);
	Add(const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<graph::Graph> >()> > generator,
	    bool onlyUniverse, IsomorphismPolicy graphPolicy);
	virtual ~Add() override;
	virtual Strategy *clone() const override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::Rules::Real &)> f) const override {}
	virtual void printInfo(PrintSettings settings) const override;
	virtual bool isConsumed(const lib::Graph::Single *g) const override;
private:
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	const std::vector<std::shared_ptr<graph::Graph> > graphs;
	const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<graph::Graph> >()> > generator;
	const bool onlyUniverse;
	const IsomorphismPolicy graphPolicy;
};

} // namespace Strategies
} // namespace DG
} // namespace lib
} // namespace mod

#endif /* MOD_LIB_DG_STRATEGIES_ADD_H */

