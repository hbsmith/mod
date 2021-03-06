include("xx0_helpers.py")

g = smiles('O')
v = DGVertex()

dg =  DG()

msg = "The DG neither has an active builder nor is locked yet."
fail(lambda: dg.findVertex(g), msg)
fail(lambda: dg.findEdge([v], [v]), msg)
fail(lambda: dg.findEdge([g], [g]), msg)

def active():
	assert dg.findVertex(g) == DGVertex()
	fail(lambda: dg.findEdge([v], []), "Source vertex descriptor is null.")
	fail(lambda: dg.findEdge([], [v]), "Target vertex descriptor is null.")
	fail(lambda: dg.findEdge([g], []), "Source vertex descriptor is null.")
	fail(lambda: dg.findEdge([], [g]), "Target vertex descriptor is null.")

b = dg.build()
# Active builder, not locked
active()

del b
# Locked, no active builder
active()
