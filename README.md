# 6.S081note

## 环境配置
* 官方文档
https://pdos.csail.mit.edu/6.S081/2020/tools.html
* 在下载risc-v toolchain时下载过慢，参考了https://zhayujie.com/mit6828-env.html
* 其中，百度云下载下来的gnu-toolchain编译时出现问题：
  ```
  Undefined symbols for architecture arm64:
  ...
  ```
  参考了https://github.com/ReZeroS/mit6.828-note/issues/2得以解决
* 目前，编译完过程中还是会出现`clang: error: the clang compiler does not support '-march=rv64imafdc'`


## 环境配置（remake）
在macos上失败了很多次，重新在docker上配置环境

参考了https://zhuanlan.zhihu.com/p/377466759
过程无比的流畅:)
