include("../smiles/smiles_cansmi_roundtrip.py")
include("../smiles/smiles_nci.py")
sys.exit(0)
for a in inputGraphs:
	postSection(a.name)
	for v in a.vertices:
		try:
			v.printStereo()
		except StereoDeductionError:
			print(a.smiles)
			raise
		except FatalError:
			print(a.smiles)
			a.print()
			raise
