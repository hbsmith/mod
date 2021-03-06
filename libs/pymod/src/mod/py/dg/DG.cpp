#include <mod/py/Common.hpp>

#include <mod/Derivation.hpp>
#include <mod/dg/Builder.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/graph/Printer.hpp>

namespace mod {
namespace dg {
namespace Py {
namespace {

DG::HyperEdge (DG::*findEdgeVertices)(const std::vector<DG::Vertex> &,
                                      const std::vector<DG::Vertex> &) const = &DG::findEdge;
DG::HyperEdge (DG::*findEdgeGraphs)(const std::vector<std::shared_ptr<mod::graph::Graph> > &,
                                    const std::vector<std::shared_ptr<mod::graph::Graph> > &) const = &DG::findEdge;

// see https://stackoverflow.com/questions/19062657/is-there-a-way-to-wrap-the-function-return-value-object-in-python-using-move-i

std::shared_ptr<Builder> DG_build(std::shared_ptr<DG> dg_) {
	return std::make_shared<Builder>(dg_->build());
}

} // namespace

void DG_doExport() {
	// rst: .. py:class:: DG
	// rst:
	// rst:		The derivation graph class. A derivation graph is a directed multi-hypergraph
	// rst:		:math:`\mathcal{H} = (V, E)`. Each hyperedge :math:`e\in E` is thus an ordered pair
	// rst:		:math:`(e^+, e^-)` of multisets of vertices, the sources and the targets.
	// rst:		Each vertex is annotated with a graph, and each hyperedge is annotated with list of transformation rules.
	// rst:		A derivation graph is constructed incrementally using a :class:`DGBuilder` obtained from the :meth:`build()`
	// rst:		function. When the obtained builder is destructed the derivation graph becomes locked and can no longer be modified.
	// rst:
	py::class_<DG, std::shared_ptr<DG>, boost::noncopyable>("DG", py::no_init)
			// rst:		.. py:method:: __init__(*, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism), \
			// rst:		                        graphDatabase=[], \
			// rst:		                        graphPolicy=IsomorphismPolicy.Check)
			// rst:
			// rst:			Create an empty unlocked derivation graph object.
			// rst:
			// rst:			:param LabelSettings labelSettings: defines which category the derivation graph object works in.
			// rst:				All morphism calculations (monomorphism and isomorphism) are thus defined by the :class:`LabelType`,
			// rst:				while the :class:`LabelRelation` is used for for monomorphism enumeration.
			// rst:			:param graphDatabase: an initial graph database.
			// rst:				Any subsequently added or constructed graph for this object
			// rst:				will be checked for isomorphism against the graph database.
			// rst:			:type graphDatabase: list[Graph]
			// rst:			:param IsomorphismPolicy graphPolicy: the policy for how the graphs of ``graphDatabase`` are checked for
			// rst:				isomorphism against each other initially. Only use ``IsomorphismPolicy.TrustMe`` if you are
			// rst:				absolutely sure that the graphs are unique up to isomorphism.
			// rst:			:raises: :class:`LogicError` if ``graphPolicy == IsomorphismPolicy.Check`` and two graphs
			// rst:				in ``graphDatabase`` are different objects but represents isomorphic graphs.
			.def("__init__", py::make_constructor(&DG::make))
					// rst:		.. py:attribute:: id
					// rst:
					// rst:			The unique instance id among all :class:`DG` objects.
					// rst:
					// rst:			:type: int
			.add_property("id", &DG::getId)
			.def(str(py::self))
					// rst:		.. py:attribute:: labelSettings
					// rst:
					// rst:			(Read-only) The label settings for the derivation graph.
					// rst:
					// rst:			:type: LabelSettings
			.add_property("labelSettings", &DG::getLabelSettings)
					//------------------------------------------------------------------
					// rst:		.. py:attribute:: hasActiveBuilder
					// rst:
					// rst:			(Read-only) Whether :meth:`build` has been called and the returned :class:`DGBuilder` is still active.
					// rst:
					// rst:			:type: bool
			.add_property("hasActiveBuilder", &DG::hasActiveBuilder)
					// rst:		.. py:attribute:: locked
					// rst:
					// rst:			(Read-only) Whether the derivation graph is locked or not.
					// rst:
					// rst:			:type: bool
			.add_property("locked", &DG::isLocked)
					//------------------------------------------------------------------
					// rst:		.. py:attribute:: numVertices
					// rst:
					// rst:			(Read-only) The number of vertices in the derivation graph.
					// rst:
					// rst:			:type: int
					// rst:			:raises: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
			.add_property("numVertices", &DG::numVertices)
					// rst:		.. py:attribute:: vertices
					// rst:
					// rst:			(Read-only) An iterable of all vertices in the derivation graph.
					// rst:
					// rst:			:type: DGVertexRange
					// rst:			:raises: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
			.add_property("vertices", &DG::vertices)
					// rst:		.. py:attribute:: numEdges
					// rst:
					// rst:			(Read-only) The number of hyperedges in the derivation graph.
					// rst:
					// rst:			:type: int
					// rst:			:raises: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
			.add_property("numEdges", &DG::numEdges)
					// rst:		.. py:attribute:: edges
					// rst:
					// rst:			(Read-only) An iterable of all hyperedges in the derivation graph.
					// rst:
					// rst:			:type: DGEdgeRange
					// rst:			:raises: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
			.add_property("edges", &DG::edges)
					//------------------------------------------------------------------
					// rst:		.. py:method:: findVertex(g)
					// rst:
					// rst:			:param Graph g: the graph to find a vertex which has it associated.
					// rst:			:returns: a vertex descriptor for which the given graph is associated,
					// rst:				or a null descriptor if no such vertex exists.
					// rst:			:rtype: DGVertex
					// rst:			:raises: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
			.def("findVertex", &DG::findVertex)
					// rst:		.. py:method:: findEdge(sources, targets)
					// rst:		               findEdge(sourceGraphs, targetGraphs)
					// rst:
					// rst:			:param sources: the list of source vertices the resulting hyperedge must have.
					// rst:			:type sources: list[DGVertex]
					// rst:			:param targets: the list of targets vertices the resulting hyperedge must have.
					// rst:			:type targets: list[DGVertex]
					// rst:			:param sourceGraphs: the list of graphs that must be associated with the source vertices the resulting hyperedge must have.
					// rst:			:type sourceGraphs: list[Graph]
					// rst:			:param targetGraphs: the list of graphs that must be associated with the targets vertices the resulting hyperedge must have.
					// rst:			:type targetGraphs: list[Graph]
					// rst:			:returns: a hyperedge with the given sources and targets.
					// rst:			  If no such hyperedge exists in the derivation graph then a null edge is returned.
					// rst:			  In the second version, the graphs are put through :meth:`findVertex` first.
					// rst:			:raises: :class:`LogicError` if a vertex descriptor is null, or does not belong to the derivation graph.
					// rst:			:raises: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
			.def("findEdge", findEdgeVertices)
			.def("findEdge", findEdgeGraphs)
					//------------------------------------------------------------------
					// rst:		.. py:method:: build()
					// rst:
					// rst:			:returns: an RAII-style object which can be used to construct the derivation graph.
					// rst:				It can be used as a context manager in a ``with``-statement (see the documentation of :class:`DGBuilder`).
					//	rst:				On destruction of an active builder object the associated DG object becomes locked for further modifications.
					// rst:			:rtype: DGBuilder
					// rst:			:raises: :class:`LogicError` if the DG already has an active builder (see :attr:`hasActiveBuilder`).
					// rst:			:raises: :class:`LogicError` if the DG is locked (see :attr:`locked`).
			.def("build", &DG_build)
					// rst:		.. py:attribute:: graphDatabase
					// rst:
					// rst:			All graphs known to the derivation graph.
					// rst:
					// rst:			:type: list[Graph]
			.add_property("_graphDatabase",
			              py::make_function(&DG::getGraphDatabase, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. py:attribute:: products
					// rst:
					// rst:			The subset of the vertex graphs which were discovered by the calculation.
					// rst:
					// rst:			:type: list[Graph]
			.add_property("_products",
			              py::make_function(&DG::getProducts, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. py:method:: print(printer=DGPrinter(), data=None)
					// rst:
					// rst:			Print the derivation graph in style of a hypergraph. The appearance and structure
					// rst:			of the visualisation can optionally be configured by giving a DG printer and/or data object.
					// rst:
					// rst:			:param DGPrinter printer: the printer to use governing the appearance.
					// rst:			:param DGPrintData data: the extra data to use encoding the structure of the graph.
					// rst:			:returns: the name of the PDF-file that will be compiled in post-processing and the name of the coordinate tex-file used.
					// rst:			:rtype: tuple[str, str]
			.def("print", &DG::print)
					// rst:		.. py:method:: dump()
					// rst:
					// rst:			Export the derivation graph to an external file. The vertex graphs are exported as well.
					// rst:
					// rst:			:returns: the filename of the exported derivation graph.
					// rst:			:rtype: str
					// rst:			:raises: :py:class:`LogicError` if the DG has not been calculated.
			.def("dump", &DG::dump)
					// rst:		.. py:method:: listStats()
					// rst:
					// rst:			Lists various statistics for the derivation graph.
					// rst:
					// rst:			:raises: :py:class:`LogicError` if the DG has not been calculated.
			.def("listStats", &DG::listStats);

	// rst: .. py:method:: dgDump(graphs, rules, file)
	// rst:
	// rst:		Load a derivation graph dump.
	// rst:
	// rst:		:param graphs: Any graph in the dump which is isomorphic one of these graphs is replaced by the given graph.
	// rst:		:type graphs: list[Graph]
	// rst:		:param rules: As for the graphs the same procedure is done for the rules, however only using the name of the rule for comparison.
	// rst:		:type rules: list[Rule]
	// rst:		:returns: the loaded derivation graph.
	// rst:		:rtype: DG
	py::def("dgDump", &DG::dumpImport);

	// rst: .. py:method:: diffDGs(dg1, dg2)
	// rst:
	// rst:		Compare two derivation graphs and lists the difference.
	// rst:		This is not a general isomorphism check; two vertices are equal if they have
	// rst:		the same graph attached. Edges are equal if the head and tail sets are equal
	// rst:		and if the attached rule is the same.
	// rst:
	// rst:		:param dg1: the first derivation graph.
	// rst:		:type dg1: :class:`DG`
	// rst:		:param dg2: the second derivation graph.
	// rst:		:type dg2: :class:`DG`
	py::def("diffDGs", &DG::diff);
}

} // namespace Py
} // namespace dg
} // namespace mod