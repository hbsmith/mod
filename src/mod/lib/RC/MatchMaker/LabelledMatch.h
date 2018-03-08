#ifndef MOD_LIB_RC_MATCH_MAKER_LABELLED_MATCH_H
#define MOD_LIB_RC_MATCH_MAKER_LABELLED_MATCH_H

#include <mod/Config.h>
#include <mod/Error.h>

#include <mod/lib/GraphMorphism/LabelledMorphism.h>
#include <mod/lib/GraphMorphism/StereoVertexMap.h>
#include <mod/lib/GraphMorphism/TermVertexMap.h>
#include <mod/lib/IO/Rule.h>

#include <jla_boost/graph/morphism/models/PropertyVertexMap.hpp>

namespace mod {
namespace lib {
namespace RC {
namespace detail {

void initTermLabel(const auto &rFirst, const auto &rSecond) {
	const auto &termFirst = get_term(rFirst.getDPORule());
	const auto &termSecond = get_term(rSecond.getDPORule());
	if(!isValid(termFirst)) {
		std::string msg = "Term state of rFirst is invalid:\n" + termFirst.getParsingError();
		lib::IO::Rules::Write::summary(rFirst);
		throw mod::FatalError(std::move(msg));
	}
	if(!isValid(termSecond)) {
		std::string msg = "Term state of rSecond is invalid:\n" + termSecond.getParsingError();
		lib::IO::Rules::Write::summary(rSecond);
		throw mod::FatalError(std::move(msg));
	}
}

void initStereoLabel(const auto &rFirst, const auto &rSecond) {
	get_stereo(rFirst.getDPORule());
	get_stereo(rSecond.getDPORule());
}

} // namespace detail

void initByLabelSettings(const auto &rFirst, const auto &rSecond, LabelSettings labelSettings) {
	switch(labelSettings.type) {
	case LabelType::String: break;
	case LabelType::Term:
		detail::initTermLabel(rFirst, rSecond);
		break;
	}
	if(labelSettings.withStereo) detail::initStereoLabel(rFirst, rSecond);
}

namespace detail {

//template<typename VertexMap>
//bool handleMapFinallyDoIt(const auto &rFirst, const auto &rSecond, VertexMap &&m, auto &&mr) {
//	return mr(rFirst, rSecond, std::move(m));
//}

template<typename VertexMap>
bool handleMapToStereo(const auto &rFirst, const auto &rSecond, VertexMap &&m, auto &&mr, LabelSettings labelSettings) {
	if(!labelSettings.withStereo) {
		return mr(rFirst, rSecond, std::move(m));
	}
	const auto &lgDom = get_labelled_left(rSecond.getDPORule());
	const auto &lgCodom = get_labelled_right(rFirst.getDPORule());
	auto mrInner = [&](auto &&mInner, const auto&, const auto&) {
		return mr(rFirst, rSecond, std::move(mInner));
	};
	switch(labelSettings.stereoRelation) {
	case LabelRelation::Isomorphism:
	{
		MOD_ABORT;
	}
	case LabelRelation::Specialisation:
	{
		auto mrStereo = lib::GraphMorphism::Stereo::makeToVertexMapSpec(lgDom, lgCodom, mrInner);
		return mrStereo(std::move(m), get_graph(lgDom), get_graph(lgCodom));
	}
	case LabelRelation::Unification:
	{
		MOD_ABORT;
	}
	}
	MOD_ABORT;
}

template<typename VertexMap>
bool handleMapToTerm(const auto &rFirst, const auto &rSecond, VertexMap &&m, auto &&mr, LabelSettings labelSettings) {
	namespace GM = jla_boost::GraphMorphism;
	using GraphDom = typename GM::VertexMapTraits<VertexMap>::GraphDom;
	using GraphCodom = typename GM::VertexMapTraits<VertexMap>::GraphCodom;

	const auto &gDom = get_graph(get_labelled_left(rSecond.getDPORule()));
	const auto &gCodom = get_graph(get_labelled_right(rFirst.getDPORule()));
	const auto vNullDom = boost::graph_traits<GraphDom>::null_vertex();
	const auto &termDom = get_term(get_labelled_left(rSecond.getDPORule()));
	const auto &termCodom = get_term(get_labelled_right(rFirst.getDPORule()));

	lib::Term::Wam machine = getMachine(termCodom);
	machine.setTemp(getMachine(termDom));
	machine.verify();
	lib::Term::MGU mgu(machine.getHeap().size());
	// first unify mapped vertices and edges
	for(const auto vCodom : asRange(vertices(gCodom))) {
		const auto vDom = get_inverse(m, gDom, gCodom, vCodom);
		if(vDom == vNullDom) continue;
		const auto addrCodom = termCodom[vCodom];
		const auto addrDom = termDom[vDom];
		machine.unifyHeapTemp(addrCodom, addrDom, mgu);
		if(mgu.status != lib::Term::MGU::Status::Exists) break;
		machine.verify();
	}
	for(const auto eCodom : asRange(edges(gCodom))) {
		if(mgu.status != lib::Term::MGU::Status::Exists) break;
		machine.verify();
		const auto vSrcCodom = source(eCodom, gCodom);
		const auto vTarCodom = target(eCodom, gCodom);
		const auto vSrcDom = get_inverse(m, gDom, gCodom, vSrcCodom);
		const auto vTarDom = get_inverse(m, gDom, gCodom, vTarCodom);
		if(vSrcDom == vNullDom) continue;
		if(vTarDom == vNullDom) continue;
		const auto epDom = edge(vSrcDom, vTarDom, gDom);
		if(!epDom.second) continue;
		const auto eSecond = epDom.first;
		const auto addrCodom = termCodom[eCodom];
		const auto addrDom = termDom[eSecond];
		machine.unifyHeapTemp(addrCodom, addrDom, mgu);
	}

	if(mgu.status != lib::Term::MGU::Status::Exists) {
		return true;
	}
	// we copy the unmatched in the visitor, so we are done now

	machine.verify();
	//		lib::IO::Rules::Write::termState(rFirst);
	//		lib::IO::Rules::Write::termState(rSecond);
	//		lib::IO::Term::Write::wam(termFirst.getMachine(), lib::Term::getStrings(), lib::IO::log() << "MachineFirst:\n");
	//		lib::IO::Term::Write::wam(termSecond.getMachine(), lib::Term::getStrings(), lib::IO::log() << "MachineSecond:\n");
	lib::GraphMorphism::TermData data{std::move(machine), std::move(mgu)};
	auto mTerm = GM::addProp(std::move(m), lib::GraphMorphism::TermDataT(), std::move(data));
	return handleMapToStereo(rFirst, rSecond, std::move(mTerm), mr, labelSettings);
}

template<typename VertexMap>
bool handleMapToStringOrTerm(const auto &rFirst, const auto &rSecond, VertexMap &&m, auto &&mr, LabelSettings labelSettings) {
	switch(labelSettings.type) {
	case LabelType::String:
		return handleMapToStereo(rFirst, rSecond, std::move(m), std::move(mr), labelSettings);
	case LabelType::Term:
		return handleMapToTerm(rFirst, rSecond, std::move(m), std::move(mr), labelSettings);
	}
	MOD_ABORT;
}

} // namesapce detail

template<typename VertexMap, typename MR>
bool handleMapByLabelSettings(const auto &rFirst, const auto &rSecond, VertexMap &&m, MR &&mr, LabelSettings labelSettings) {
	return detail::handleMapToStringOrTerm(rFirst, rSecond, std::forward<VertexMap>(m), std::forward<MR>(mr), labelSettings);
}

} // namespace RC
} // namespace lib
} // namespace mod

#endif /* MOD_LIB_RC_MATCH_MAKER_LABELLED_MATCH_H */