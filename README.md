[//]: # (<img src="resource/RimeDictHelper2.png" width="300"  alt="RimeDictHelper2.png"/>)

# RimeDictHelper forLinux

RimeDictHelper2的linux版本，功能和使用方法可参考[Windows版使用文档，功能相同。](https://github.com/Sh1kaCodAyo/RimeDictHelper2)此处提供编译指南

## 🛠️ 编译与运行指南

本项目使用 C++ 和 gtkmm 编写，依赖 CMake 进行构建。

### 1. 安装编译

> **提示**：如果你的系统之前已经配置过 C++ 与 gtkmm 的开发环境，可以跳过这一步。如果是全新安装的系统，请根据你的发行版执行对应命令：

**基于 Debian/Ubuntu:**

```bash
sudo apt update
sudo apt install build-essential cmake libgtkmm-3.0-dev
```

**基于 Fedora:**

```bash
sudo dnf install gcc-c++ cmake gtkmm30-devel
```

**基于 Arch Linux:**

```bash
sudo pacman -S base-devel cmake gtkmm3
```

### 2. 命令行编译

准备好环境后，在项目根目录下执行以下命令：

```bash
# 创建构建目录并进入
mkdir -p build && cd build

# 生成构建文件
cmake -DCMAKE_BUILD_TYPE=Release ..

# 开始编译 (此处使用 4 个线程加速)
make -j4
```

编译成功后，在 build 目录下会生成可执行文件 RimeDictHelperLinux。

### 3. 准备运行环境

由于程序需要调用外部脚本进行部署（若需要完成其它功能如 Git 同步，也通过脚本实现），请确保 RimeDictHelperLinux 可执行文件、
RimeDictHelper2.png（图标）以及 deploy.sh（部署脚本）放置在同一个目录下。 赋予脚本执行权限

```bash
chmod +x deploy.sh
```

### 4. 运行程序

```bash
./RimeDictHelperLinux
```
