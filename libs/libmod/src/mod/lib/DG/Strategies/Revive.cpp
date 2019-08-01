#include "Revive.hpp"

#include <mod/Config.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/Graph/Single.hpp>

#include <iostream>

namespace mod {
namespace lib {
namespace DG {
namespace Strategies {

Revive::Revive(Strategy *strat) : Strategy(strat->getMaxComponents()), strat(strat) { }

Revive::~Revive() {
	delete strat;
}

Strategy *Revive::clone() const {
	return new Revive(strat->clone());
}

void Revive::preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>) > add) const {
	strat->preAddGraphs(add);
}

void Revive::forEachRule(std::function<void(const lib::Rules::Real&)> f) const {
	strat->forEachRule(f);
}

void Revive::printInfo(std::ostream &s) const {
	s << indent << "Revive:" << std::endl;
	indentLevel++;
	strat->printInfo(s);
	printBaseInfo(s);
	s << indent << "revived =";
	for(const Graph::Single *g : revivedGraphs) s << " " << g->getName();
	s << std::endl;
	indentLevel--;
}

bool Revive::isConsumed(const Graph::Single *g) const {
	return strat->isConsumed(g);
}

void Revive::setExecutionEnvImpl() {
	strat->setExecutionEnv(getExecutionEnv());
}

void Revive::executeImpl(std::ostream &s, const GraphState &input) {
	if(getConfig().dg.calculateVerbose.get()) {
		s << indent << "Revive:" << std::endl;
		indentLevel++;
	}
	strat->execute(s, input);
	output = new GraphState(strat->getOutput());
	assert(*output == strat->getOutput());

	for(const Graph::Single *g : input.getSubset(0)) {
		if(!strat->isConsumed(g) && output->isInUniverse(g)) {
			output->addToSubset(0, g);
			revivedGraphs.push_back(g);
		}
	}
	if(getConfig().dg.calculateVerbose.get()) indentLevel--;
}

} // namespace Strategies
} // namespace DG
} // namespace lib
} // namespace mod
