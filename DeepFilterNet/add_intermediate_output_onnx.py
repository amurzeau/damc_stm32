import onnx
import os
from onnxconverter_common import float16

dirname = os.path.dirname(__file__)


def modify_model_test():
    value_info_protos = []
    onnx_model = onnx.load(os.path.join(
        dirname, "df_dec_preprocess_2_preprocess.onnx"))
    shape_info = onnx.shape_inference.infer_shapes(onnx_model)
    for idx, node in enumerate(shape_info.graph.value_info):
        if "Conv" in node.name or "Einsum" in node.name or "GRU" in node.name or "Relu" in node.name:
            print(idx, node)
            value_info_protos.append(node)
    # in inference stage, these tensor will be added to output dict.
    onnx_model.graph.output.extend(value_info_protos)
    onnx_model = float16.convert_float_to_float16(onnx_model)
    onnx.checker.check_model(onnx_model)
    onnx.save(onnx_model, os.path.join(dirname, 'test.onnx'))
    exit()


modify_model_test()
