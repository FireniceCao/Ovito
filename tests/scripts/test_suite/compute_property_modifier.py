import ovito
import ovito.io
import ovito.modifiers

import numpy

node = ovito.io.import_file("../../files/CFG/shear.void.120.cfg")
modifier = ovito.modifiers.ComputePropertyModifier()
assert(len(modifier.expressions) == 1)

print(modifier.expressions)
print(modifier.only_selected)
print(modifier.output_property)

modifier.expressions = ["Position.X * 2.0", "Position.Y / 2"]
modifier.only_selected = False

print("Expressions: {}".format(list(modifier.expressions)))
node.modifiers.append(modifier)

modifier = ovito.modifiers.ComputePropertyModifier()
modifier.output_property = "Color"
modifier.expressions = ["Position.X / CellSize.X", "0", "0"]
node.modifiers.append(modifier)

print(node.compute()["Custom property"].array)
print(node.compute().position.array)
print(node.compute().color.array)

assert(node.compute()["Custom property"].array[0,0] == node.compute().position.array[0,0] * 2.0)