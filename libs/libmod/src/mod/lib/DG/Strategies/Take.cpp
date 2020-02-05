#include "Take.hpp"

#include <mod/Config.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>

#include <iostream>
#include <unordered_set>

namespace mod {
namespace lib {
namespace DG {
namespace Strategies {

Take::Take(unsigned int limit, bool doUniverse) : Strategy(0), limit(limit), doUniverse(doUniverse) { }

Take::~Take() = default;

Strategy *Take::clone() const {
	return new Take(limit, doUniverse);
}

void Take::preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>, IsomorphismPolicy)> add) const { }

void Take::printInfo(PrintSettings settings) const {
	settings.indent() << "Take";
	std::ostream &s = settings.s;
	if(doUniverse) s << " of universe";
	s << '\n';
	++settings.indentLevel;
	settings.indent() << "limit = " << limit << '\n';
	printBaseInfo(settings);
}

bool Take::isConsumed(const Graph::Single *g) const {
	return false;
}

void Take::executeImpl(PrintSettings settings, const GraphState &input) {
	if(getConfig().dg.calculateVerbose.get()) {
		settings.indent() << "Take: " << std::endl;
		++settings.indentLevel;
	}
	assert(!output);
	if(doUniverse) {
		output = new GraphState();
		std::unordered_set<const Graph::Single*> copiedToOutput;
		for(unsigned int i = 0; i < limit && i < input.getUniverse().size(); i++) {
			output->addToUniverse(input.getUniverse()[i]);
			copiedToOutput.insert(input.getUniverse()[i]);
		}

		for(const GraphState::SubsetStore::value_type &subset : input.getSubsets())
			for(const Graph::Single *g : subset.second) {
				std::unordered_set<const Graph::Single*>::const_iterator iter = copiedToOutput.find(g);
				if(iter != copiedToOutput.end()) {
					assert(output->isInUniverse(g));
					output->addToSubset(subset.first, g);
				}
			}
	} else {
		output = new GraphState(input.getUniverse());

		for(const GraphState::SubsetStore::value_type &p : input.getSubsets()) {
			if(p.first != 0) { // TODO: change 0 to the subset index
				assert(false);

				for(const Graph::Single *g : p.second) {
					// TODO: this could be faster if we had access to the individual indices, as we could map old to new
					assert(output->isInUniverse(g));
					output->addToSubset(p.first, g);
				}
			} else {
				for(unsigned int i = 0; i < limit && i < p.second.size(); i++) {
					assert(output->isInUniverse(*(p.second.begin() + i)));
					output->addToSubset(p.first, *(p.second.begin() + i));
				}
			}
		}
	}
	if(getConfig().dg.calculateVerbose.get())
		--settings.indentLevel;
}

} // namespace Strategies
} // namespace DG
} // namespace lib
} // namespace mod