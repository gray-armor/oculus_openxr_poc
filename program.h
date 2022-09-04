#pragma once

struct IOpenXrProgram {
  virtual ~IOpenXrProgram() = default;

  virtual void CreateInstance() = 0;

  virtual void InitializeSystem() = 0;

//  virtual void InitializeSession() = 0;
//
//  virtual void CreateSwapChains() = 0;
};

std::shared_ptr<IOpenXrProgram> CreateOpenXrProgram(const std::shared_ptr<IGraphicsPlugin> &graphicPlugin,
                                                    const std::shared_ptr<IPlatformPlugin> &platformPlugin);