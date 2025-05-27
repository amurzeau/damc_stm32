import onnx
from onnxconverter_common import float16

model = onnx.load("denoiser_LSTM_Valetini_nobatchsize.onnx")
model_fp16 = float16.convert_float_to_float16(model)
onnx.save(model_fp16, "denoiser_LSTM_Valetini_nobatchsize_fp16.onnx")
