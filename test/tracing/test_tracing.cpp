#include <ATen/ATen.h>
#include <torch/script.h>
#include <torch/torch.h>
#include <torchvision/cpu/vision_cpu.h>
#include <torchvision/ROIPool.h>
#include <torchvision/nms.h>


int main() {
  torch::DeviceType device_type;
  device_type = torch::kCPU;

  torch::jit::script::Module module;
  try {
    std::cout << "Loading model" << std::endl;
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load("yolov5s.torchscript.pt");
    std::cout << "Model loaded" << std::endl;
  } catch (const torch::Error& e) {
    std::cout << "error loading the model" << std::endl;
    return -1;
  } catch (const std::exception& e) {
    std::cout << "Other error: " << e.what() << std::endl;
    return -1;
  }

  // TorchScript models require a List[IValue] as input
  std::vector<torch::jit::IValue> inputs;

  // Demonet accepts a List[Tensor] as main input
  torch::Tensor images = torch::rand({1, 3, 416, 352});

  inputs.push_back(images);
  auto output = module.forward(inputs);

  auto detections = output.toTuple()->elements()[0];

  std::cout << "ok, detections: " << detections << std::endl;

  if (torch::cuda::is_available()) {
    // Move traced model to GPU
    module.to(torch::kCUDA);

    // Add GPU inputs
    inputs.clear();

    torch::TensorOptions options = torch::TensorOptions{torch::kCUDA};
    images = images.to(torch::kCUDA); 

    inputs.push_back(images);
    auto output = module.forward(inputs);

    auto detections = output.toTuple()->elements()[0];

    std::cout << "ok, detections: " << detections << std::endl;
  }
  return 0;
}
