#include "Add.hpp"

#include <mod/Function.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/lib/DG/NonHyperRuleComp.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>

namespace mod {
namespace lib {
namespace DG {
namespace Strategies {

Add::Add(const std::vector<std::shared_ptr<graph::Graph> > graphs, bool onlyUniverse)
: Strategy::Strategy(0), graphs(graphs), generator(nullptr), onlyUniverse(onlyUniverse) { }

Add::Add(const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<graph::Graph> >() > > generator, bool onlyUniverse)
: Strategy::Strategy(0), graphs(), generator(generator), onlyUniverse(onlyUniverse) { }

Add::~Add() { }

Strategy *Add::clone() const {
	if(!generator) return new Add(graphs, onlyUniverse);
	else return new Add(generator, onlyUniverse);
}

void Add::preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>) > add) const {
	for(const auto &g : graphs) add(g);
}

void Add::printInfo(std::ostream &s) const {
	s << indent << "Add";
	if(onlyUniverse) s << "Universe";
	s << ":";
	if(generator) {
		indentLevel++;
		s << std::endl << indent << "function = ";
		generator->print(s);
		indentLevel--;
	} else {
		for(const std::shared_ptr<graph::Graph> g : graphs) s << " " << g->getName();
	}
	s << std::endl;
	indentLevel++;
	printBaseInfo(s);
	indentLevel--;
}

bool Add::isConsumed(const lib::Graph::Single *g) const {
	return false;
}

void Add::executeImpl(std::ostream &s, const GraphState &input) {
	std::vector<std::shared_ptr<graph::Graph> > graphsToAdd;
	if(generator) {
		graphsToAdd = (*generator)();
		if(!getConfig().dg.skipInitialGraphIsomorphismCheck.get())
			for(const auto &gCand : graphsToAdd)
				getExecutionEnv().tryAddGraph(gCand);
	} else {
		graphsToAdd = graphs;
	}
	for(const std::shared_ptr<graph::Graph> g : graphsToAdd) getExecutionEnv().trustAddGraphAsVertex(g);
	output = new GraphState(input);
	if(onlyUniverse) for(const std::shared_ptr<graph::Graph> g : graphsToAdd) output->addToUniverse(&g->getGraph());
	else {
		for(const std::shared_ptr<graph::Graph> g : graphsToAdd) output->addToSubset(0, &g->getGraph());
	}
}

} // namespace Strategies
} // namespace DG
} // namespace lib
} // namespace mod