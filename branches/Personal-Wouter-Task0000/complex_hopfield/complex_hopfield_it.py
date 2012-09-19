import numpy

def calculate_outputs(activation, steepness):
    outputs = []
    for act in activation:
        outputs.append(numpy.tanh(steepness * act))
    return outputs

def test_calculate_outputs():
    activation = [numpy.complex(1, 0), numpy.complex(-1, 0)]
    steepness = 1.0
    print  calculate_outputs(activation, steepness)

def calculate_network_activity(weights, outputs):
    net_act = []
    for idx, out in enumerate(outputs):
        act = 0
        for idy in range(len(outputs)):
            act += out * weights[idx][idy]
        net_act.append(act)
    return net_act


weights = [[numpy.complex(1, 0), numpy.complex(-1, 0)],
           [numpy.complex(-1, 0), numpy.complex(1, 0)]]

activation = [numpy.complex(1, 0), numpy.complex(-1, 0)]
steepness = 1.0
outputs = calculate_outputs(activation, steepness)
print outputs
net_acts = calculate_network_activity(weights, outputs)
print net_acts
