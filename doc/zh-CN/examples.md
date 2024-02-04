# 示例
注意：需要将 `papilio_build_example` 设置为 `ON` 来构建示例。详细信息请参阅 [自定义构建](custom_build.md)。

## 编译信息
- 位于 `example/info.cpp` 中

输出库版本号，以及编译器与标准库的信息。

## 脚本演示
- 位于 `example/script_demo.cpp` 中

演示脚本中的逻辑控制如何在国际化（i18n）场景中发挥作用。

## 带样式的输出
- 位于 `example/styled_output.cpp` 中

在支持 ANSI 转义序列的终端上输出有自定义颜色和字体的字符串。

## 交互式操场
- 位于 `example/playground/` 目录中
- 二进制可执行文件名为 `ipapilio`，位于 `${CMAKE_BINARY_DIR}/example/` 目录中

用于测试格式输出的交互式程序，可以自定义格式串、参数等。在程序中输入“help”以查看支持的命令。
