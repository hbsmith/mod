include("formoseCommon/grammar.py")

dg = dgRuleComp(inputGraphs, addSubset(inputGraphs) >> inputRules[0] >> inputRules[2])
dg.calc()

printer = DGPrinter()
postSection("Const False")
printer.pushVertexVisible(False)
dg.print(printer)
printer.popVertexVisible()
postSection("Const True")
printer.pushVertexVisible(True)
dg.print(printer)
printer.popVertexVisible()

postSection("Func")
def f(g, dg): return all(g != a for a in inputGraphs)
printer.pushVertexVisible(f)
dg.print(printer)
printer.popVertexVisible()

printer.pushVertexVisible(True)
dg.print(printer)
printer.popVertexVisible()

postSection("Lambda")
printer.pushVertexVisible(lambda g, dg: all(g != a for a in inputGraphs))
dg.print(printer)
printer.popVertexVisible()
