<?xml version='1.0' encoding='utf-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
    <context>
        <name>CompileOptions</name>
        <message>
            <location filename="../core/CompileOptions.cpp" line="46" />
            <location filename="../core/CompileOptions.cpp" line="50" />
            <source>None</source>
            <translation>无</translation>
        </message>
        <message>
            <location filename="../core/CompileOptions.cpp" line="47" />
            <source>Performance -O</source>
            <translation>性能 -O</translation>
        </message>
        <message>
            <location filename="../core/CompileOptions.cpp" line="48" />
            <source>Size -Os</source>
            <translation>体积 -Os</translation>
        </message>
    </context>
    <context>
        <name>Compiler</name>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="173" />
            <source>glslang has not been initialized, so compilation is impossible.</source>
            <translation>glslang 尚未初始化，无法编译。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="174" />
            <source>Restart the application; if the problem persists, glslang::InitializeProcess() failed, so check the start-up messages in the log panel.</source>
            <translation>请重启应用程序；如果问题仍然存在，说明 glslang::InitializeProcess() 调用失败，请检查日志面板中的启动信息。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="180" />
            <source>The source is empty, so there is nothing to compile.</source>
            <translation>源代码为空，没有可编译的内容。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="181" />
            <source>Enter GLSL code first, or open a shader file from the file list on the left.</source>
            <translation>请先输入 GLSL 代码，或从左侧文件列表打开着色器文件。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="183" />
            <source>The source is empty</source>
            <translation>源代码为空</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="189" />
            <source>GLSL version %1 is not valid.</source>
            <translation>GLSL 版本 %1 无效。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="191" />
            <source>Pick a version between 100 and 460 in the version combo box on the toolbar.</source>
            <translation>请在工具栏的版本下拉框中选择 100 到 460 之间的版本。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="204" />
            <source>#include expansion failed, so compilation did not start.</source>
            <translation>#include 展开失败，编译未能开始。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="205" />
            <source>Fix the #include paths listed in the problem list below, or turn off "Enable #include" in the settings.</source>
            <translation>请修正下方问题列表中列出的 #include 路径，或在设置中关闭“Enable #include”。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="278" />
            <source>%1 shader (%2) failed to compile.</source>
            <translation>%1 着色器（%2）编译失败。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="280" />
            <source>See the problem list below: double-clicking an entry jumps to the offending line and column. Common causes are a missing semicolon, an undeclared variable, or a #version that does not match the features used.</source>
            <translation>请查看下方问题列表：双击某个条目即可跳转到出错的代码行和列。常见原因是缺少分号、变量未声明，或 #version 与所用特性不匹配。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="295" />
            <source>glslang failed to compile and gave no details.</source>
            <translation>glslang 编译失败，且未给出详细信息。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="311" />
            <source>%1 shader failed to link.</source>
            <translation>%1 着色器链接失败。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="312" />
            <source>A link failure usually means the entry point main() is missing or has the wrong signature. Check that main() is defined and takes no parameters.</source>
            <translation>链接失败通常意味着入口点 main() 缺失或签名不正确。请确认已定义 main() 且它不接受任何参数。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="329" />
            <source>glslang did not produce an intermediate representation for stage %1.</source>
            <translation>glslang 未为阶段 %1 生成中间表示。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="331" />
            <source>Make sure the Stage selected on the toolbar matches the actual stage of the source (for example, choose Vertex for a vertex shader).</source>
            <translation>请确认工具栏中选择的 Stage 与源代码的实际阶段一致（例如，顶点着色器请选择 Vertex）。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="334" />
            <source>TIntermediate is null</source>
            <translation>TIntermediate 为 null</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="361" />
            <source>glslang produced no SPIR-V words at all.</source>
            <translation>glslang 完全没有生成任何 SPIR-V 字。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="362" />
            <source>This usually means there is no compilable code in the source. Check that a main() function exists and that the Stage selected on the toolbar is correct.</source>
            <translation>这通常意味着源代码中没有可编译的代码。请确认存在 main() 函数，并且工具栏中选择的 Stage 正确。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="365" />
            <source>The generated SPIR-V is empty</source>
            <translation>生成的 SPIR-V 为空</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="379" />
            <source>The SPIR-V header produced by glslang is invalid (magic number check failed).</source>
            <translation>glslang 生成的 SPIR-V 头无效（魔数校验失败）。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="381" />
            <source>This is an internal tool failure. Please report it together with the current source and the compile log so that it can be tracked down.</source>
            <translation>这是工具内部故障。请连同当前源代码和编译日志一起反馈，以便定位该问题。</translation>
        </message>
        <message>
            <location filename="../core/GlslCompiler.cpp" line="384" />
            <source>SPIR-V magic is not 0x07230203</source>
            <translation>SPIR-V 魔数不是 0x07230203</translation>
        </message>
    </context>
    <context>
        <name>Diagnostic</name>
        <message>
            <location filename="../core/Diagnostic.cpp" line="34" />
            <location filename="../core/Diagnostic.cpp" line="78" />
            <source>GLSL syntax</source>
            <translation>GLSL 语法</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="35" />
            <location filename="../core/Diagnostic.cpp" line="79" />
            <source>Link</source>
            <translation>链接</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="36" />
            <location filename="../core/Diagnostic.cpp" line="80" />
            <source>Preprocess</source>
            <translation>预处理</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="37" />
            <location filename="../core/Diagnostic.cpp" line="81" />
            <source>SPIR-V validation</source>
            <translation>SPIR-V 校验</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="38" />
            <location filename="../core/Diagnostic.cpp" line="82" />
            <source>Optimization</source>
            <translation>优化</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="39" />
            <location filename="../core/Diagnostic.cpp" line="49" />
            <location filename="../core/Diagnostic.cpp" line="83" />
            <source>Hint</source>
            <translation>提示</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="41" />
            <source>Unknown</source>
            <translation>未知</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="46" />
            <source>Error</source>
            <translation>错误</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="47" />
            <source>Warning</source>
            <translation>警告</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="48" />
            <location filename="../core/Diagnostic.cpp" line="51" />
            <source>Info</source>
            <translation>信息</translation>
        </message>
        <message>
            <location filename="../core/Diagnostic.cpp" line="244" />
            <source>This line looks like a complete statement but does not end with ';'. Ignore it if it just continues the previous statement; otherwise add the missing semicolon.</source>
            <translation>这一行看起来是完整语句，但没有以 ';' 结尾。如果它只是上一条语句的延续，可以忽略；否则请补上缺少的分号。</translation>
        </message>
    </context>
    <context>
        <name>Disassembler</name>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="100" />
            <source>SPIRV-Tools returned an empty diagnostic message.</source>
            <translation>SPIRV-Tools 返回了空的诊断消息。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="110" />
            <source>SPIRV-Tools reported an error with no description.</source>
            <translation>SPIRV-Tools 报告了错误，但没有给出描述。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="115" />
            <source> (instruction index %1)</source>
            <translation> （指令索引 %1）</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="213" />
            <source>Unknown</source>
            <translation>未知</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="230" />
            <source>There is no SPIR-V data to disassemble.</source>
            <translation>没有可反汇编的 SPIR-V 数据。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="239" />
            <location filename="../core/SpirvDisassembler.cpp" line="289" />
            <source>Could not create a SPIRV-Tools context (target environment %1).</source>
            <translation>无法创建 SPIRV-Tools 上下文（目标环境 %1）。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="261" />
            <source>Disassembly failed: %1</source>
            <translation>反汇编失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="280" />
            <source>There is no SPIR-V data to validate.</source>
            <translation>没有可校验的 SPIR-V 数据。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="303" />
            <source>Could not create the validator options object.</source>
            <translation>无法创建校验器选项对象。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="319" />
            <source>SPIR-V validation failed: %1</source>
            <translation>SPIR-V 校验失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="344" />
            <source>This build does not link SPIRV-Tools-opt, so optimization was skipped. To enable it, make sure the SPIRV-Tools-opt target is found at build time.</source>
            <translation>本次构建未链接 SPIRV-Tools-opt，因此跳过了优化。如需启用，请确保构建时能找到 SPIRV-Tools-opt 目标。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="350" />
            <source>There is no SPIR-V data to optimize.</source>
            <translation>没有可优化的 SPIR-V 数据。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="360" />
            <source>Could not create the optimizer options object.</source>
            <translation>无法创建优化器选项对象。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="378" />
            <source>Could not create an optimizer for environment %1.</source>
            <translation>无法为环境 %1 创建优化器。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="416" />
            <source>Optimization failed: %1</source>
            <translation>优化失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="423" />
            <source>The module returned by the optimizer is too short; its result was ignored.</source>
            <translation>优化器返回的模块过短，已忽略其结果。</translation>
        </message>
        <message>
            <location filename="../core/SpirvDisassembler.cpp" line="437" />
            <source>The module produced by the optimizer has an invalid magic number, so the unoptimized result was kept.</source>
            <translation>优化器生成的模块魔数无效，因此保留了未优化的结果。</translation>
        </message>
    </context>
    <context>
        <name>EditorTab</name>
        <message>
            <location filename="../ui/EditorTab.cpp" line="397" />
            <source>Untitled</source>
            <translation>未命名</translation>
        </message>
    </context>
    <context>
        <name>FileBrowser</name>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="62" />
            <source>Shaders (%1);;SPIR-V Assembly (*.spvasm);;All Files (*)</source>
            <translation>着色器 (%1);;SPIR-V 汇编 (*.spvasm);;所有文件 (*)</translation>
        </message>
    </context>
    <context>
        <name>GlslLanguageData</name>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="45" />
            <source>Converts degrees to radians</source>
            <translation>角度转弧度</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="46" />
            <source>Converts radians to degrees</source>
            <translation>弧度转角度</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="47" />
            <source>Sine</source>
            <translation>正弦</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="48" />
            <source>Cosine</source>
            <translation>余弦</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="49" />
            <source>Tangent</source>
            <translation>正切</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="50" />
            <source>Arc sine; x must be in [-1, 1]</source>
            <translation>反正弦，x 需在 [-1,1]</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="51" />
            <source>Arc cosine; x must be in [-1, 1]</source>
            <translation>反余弦，x 需在 [-1,1]</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="52" />
            <source>Arc tangent (the two-argument form returns the correct quadrant)</source>
            <translation>反正切（双参数形式返回正确的象限）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="53" />
            <source>Hyperbolic sine</source>
            <translation>双曲正弦</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="54" />
            <source>Hyperbolic cosine</source>
            <translation>双曲余弦</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="55" />
            <source>Hyperbolic tangent</source>
            <translation>双曲正切</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="56" />
            <source>Inverse hyperbolic sine</source>
            <translation>反双曲正弦</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="57" />
            <source>Inverse hyperbolic cosine; x &gt;= 1</source>
            <translation>反双曲余弦，x &gt;= 1</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="58" />
            <source>Inverse hyperbolic tangent; |x| &lt; 1</source>
            <translation>反双曲正切，|x| &lt; 1</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="61" />
            <source>x raised to the power y</source>
            <translation>x 的 y 次幂</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="62" />
            <source>Natural exponential, e^x</source>
            <translation>自然指数 e^x</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="63" />
            <source>Natural logarithm</source>
            <translation>自然对数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="64" />
            <source>2 raised to the power x</source>
            <translation>2 的 x 次幂</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="65" />
            <source>Base-2 logarithm</source>
            <translation>以 2 为底的对数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="66" />
            <source>Square root</source>
            <translation>平方根</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="67" />
            <source>Reciprocal square root, 1/sqrt(x)</source>
            <translation>平方根倒数 1/sqrt(x)</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="70" />
            <source>Absolute value</source>
            <translation>绝对值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="71" />
            <source>Sign: -1, 0 or 1</source>
            <translation>符号：-1、0 或 1</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="72" />
            <source>Rounds down to the nearest integer</source>
            <translation>向下取整</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="73" />
            <source>Truncates towards zero</source>
            <translation>向零截断</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="74" />
            <source>Rounds to the nearest integer (tie direction is implementation-defined)</source>
            <translation>四舍五入（.5 方向由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="75" />
            <source>Rounds to the nearest integer, ties to even</source>
            <translation>四舍五入，.5 取偶数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="76" />
            <source>Rounds up to the nearest integer</source>
            <translation>向上取整</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="77" />
            <source>Fractional part, x - floor(x)</source>
            <translation>小数部分 x - floor(x)</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="78" />
            <source>Modulo, x - y*floor(x/y)</source>
            <translation>取模 x - y*floor(x/y)</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="79" />
            <source>Modulo with a scalar divisor</source>
            <translation>标量除数取模</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="80" />
            <source>Splits x into integral and fractional parts</source>
            <translation>将 x 拆分为整数和小数部分</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="81" />
            <source>Smaller of the two values</source>
            <translation>两值中的较小者</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="82" />
            <source>Larger of the two values</source>
            <translation>两值中的较大者</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="83" />
            <source>Clamps x to the range [minVal, maxVal]</source>
            <translation>将 x 钳制到 [minVal, maxVal] 范围</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="84" />
            <source>Linear interpolation</source>
            <translation>线性插值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="85" />
            <source>Linear interpolation with a scalar weight</source>
            <translation>用标量权重线性插值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="86" />
            <source>Returns 0 when x &lt; edge, otherwise 1</source>
            <translation>当 x &lt; edge 返回 0，否则返回 1</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="87" />
            <source>Smooth Hermite interpolation</source>
            <translation>平滑 Hermite 插值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="88" />
            <source>True where x is NaN</source>
            <translation>x 为 NaN 时为真</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="89" />
            <source>True where x is infinite</source>
            <translation>x 为无穷时为真</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="90" />
            <source>Reinterprets the bits as int</source>
            <translation>将位重新解释为 int</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="91" />
            <source>Reinterprets the bits as uint</source>
            <translation>将位重新解释为 uint</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="92" />
            <location filename="../ui/GlslLanguageData.cpp" line="93" />
            <source>Reinterprets the bits as float</source>
            <translation>将位重新解释为 float</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="94" />
            <source>Fused multiply-add a*b+c</source>
            <translation>融合乘加 a*b+c</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="95" />
            <source>Splits x into mantissa and exponent</source>
            <translation>将 x 拆分为尾数和指数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="96" />
            <source>x * 2^exp</source>
            <translation>x * 2^exp</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="97" />
            <source>Packs two normalized signed 16-bit values into a uint</source>
            <translation>将两个归一化有符号 16 位值打包进 uint</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="98" />
            <source>Unpacks two normalized signed 16-bit values from a uint</source>
            <translation>从 uint 解包两个归一化有符号 16 位值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="99" />
            <source>Packs two normalized unsigned 16-bit values into a uint</source>
            <translation>将两个归一化无符号 16 位值打包进 uint</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="100" />
            <source>Unpacks two normalized unsigned 16-bit values from a uint</source>
            <translation>从 uint 解包两个归一化无符号 16 位值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="101" />
            <source>Packs two half-floats into a uint</source>
            <translation>将两个半精度浮点打包进 uint</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="102" />
            <source>Unpacks two half-floats from a uint</source>
            <translation>从 uint 解包两个半精度浮点</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="103" />
            <source>Packs four normalized unsigned bytes into a uint</source>
            <translation>将四个归一化无符号字节打包进 uint</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="104" />
            <source>Unpacks four normalized unsigned bytes from a uint</source>
            <translation>从 uint 解包四个归一化无符号字节</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="105" />
            <source>Packs four normalized signed bytes into a uint</source>
            <translation>将四个归一化有符号字节打包进 uint</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="106" />
            <source>Unpacks four normalized signed bytes from a uint</source>
            <translation>从 uint 解包四个归一化有符号字节</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="109" />
            <source>Length of a vector</source>
            <translation>向量长度</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="110" />
            <source>Distance between two points</source>
            <translation>两点间距离</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="111" />
            <source>Dot product</source>
            <translation>点积</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="112" />
            <source>Cross product</source>
            <translation>叉积</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="113" />
            <source>Normalizes a vector</source>
            <translation>归一化向量</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="114" />
            <source>Flips the normal to face away from the reference direction</source>
            <translation>翻转法线使其背离参考方向</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="115" />
            <source>Reflection vector</source>
            <translation>反射向量</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="116" />
            <source>Refraction vector</source>
            <translation>折射向量</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="119" />
            <source>Component-wise multiply (not a matrix product)</source>
            <translation>逐分量相乘（非矩阵乘积）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="120" />
            <source>Outer product c * r^T</source>
            <translation>外积 c * r^T</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="121" />
            <source>Transpose</source>
            <translation>转置</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="122" />
            <source>Determinant</source>
            <translation>行列式</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="123" />
            <source>Inverse matrix</source>
            <translation>逆矩阵</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="126" />
            <source>Component-wise &lt;</source>
            <translation>逐分量 &lt;</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="127" />
            <source>Component-wise &lt;=</source>
            <translation>逐分量 &lt;=</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="128" />
            <source>Component-wise &gt;</source>
            <translation>逐分量 &gt;</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="129" />
            <source>Component-wise &gt;=</source>
            <translation>逐分量 &gt;=</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="130" />
            <source>Component-wise ==</source>
            <translation>逐分量 ==</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="131" />
            <source>Component-wise !=</source>
            <translation>逐分量 !=</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="132" />
            <source>True when any component is true</source>
            <translation>任一分量为真时为真</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="133" />
            <source>True when all components are true</source>
            <translation>所有分量为真时为真</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="134" />
            <source>Component-wise logical not</source>
            <translation>逐分量逻辑非</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="137" />
            <source>Unsigned add, also returning the carry</source>
            <translation>无符号加法，同时返回进位</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="138" />
            <source>Unsigned subtract, also returning the borrow</source>
            <translation>无符号减法，同时返回借位</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="139" />
            <source>64-bit unsigned multiply</source>
            <translation>64 位无符号乘法</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="140" />
            <source>64-bit signed multiply</source>
            <translation>64 位有符号乘法</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="141" />
            <source>Extracts a bit field</source>
            <translation>提取位域</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="142" />
            <source>Inserts a bit field</source>
            <translation>插入位域</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="143" />
            <source>Reverses the bit order</source>
            <translation>反转位序</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="144" />
            <source>Number of set bits</source>
            <translation>置位比特数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="145" />
            <source>Index of the least significant set bit</source>
            <translation>最低置位比特的索引</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="146" />
            <source>Index of the most significant set bit</source>
            <translation>最高置位比特的索引</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="149" />
            <source>Samples a texture (LOD chosen automatically)</source>
            <translation>采样纹理（自动选择 LOD）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="150" />
            <source>Samples a texture with an LOD bias</source>
            <translation>带 LOD bias 采样纹理</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="151" />
            <source>Projective texture sampling</source>
            <translation>投影纹理采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="152" />
            <source>Samples at an explicit LOD</source>
            <translation>按指定 LOD 采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="153" />
            <source>Samples with a texel offset</source>
            <translation>带纹素偏移采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="154" />
            <source>Fetches a texel by integer coordinates</source>
            <translation>按整数坐标取纹素</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="155" />
            <source>Fetches a texel by integer coordinates plus an offset</source>
            <translation>按整数坐标加偏移取纹素</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="156" />
            <source>Projective sampling with an offset</source>
            <translation>带偏移的投影采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="157" />
            <source>Samples at an explicit LOD with an offset</source>
            <translation>按指定 LOD 带偏移采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="158" />
            <source>Projective sampling at an explicit LOD</source>
            <translation>按指定 LOD 的投影采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="159" />
            <source>Projective sampling at an explicit LOD with an offset</source>
            <translation>按指定 LOD 带偏移的投影采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="160" />
            <source>Samples with explicit gradients</source>
            <translation>用显式梯度采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="161" />
            <source>Samples with explicit gradients and an offset</source>
            <translation>用显式梯度带偏移采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="162" />
            <source>Projective sampling with explicit gradients</source>
            <translation>用显式梯度的投影采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="163" />
            <source>Projective sampling with explicit gradients and an offset</source>
            <translation>用显式梯度带偏移的投影采样</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="164" />
            <source>Gathers four neighbouring texels</source>
            <translation>聚集四个相邻纹素</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="165" />
            <source>Gathers four neighbouring texels with an offset</source>
            <translation>带偏移聚集四个相邻纹素</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="166" />
            <source>Gathers texels using four offsets</source>
            <translation>用四个偏移聚集纹素</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="167" />
            <source>Queries the texture size</source>
            <translation>查询纹理尺寸</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="168" />
            <source>Queries the LOD that would be used</source>
            <translation>查询将使用的 LOD</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="169" />
            <source>Queries the number of mip levels</source>
            <translation>查询 mip 层级数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="170" />
            <source>Queries the number of samples</source>
            <translation>查询采样数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="173" />
            <source>Atomic add</source>
            <translation>原子加</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="174" />
            <source>Atomic minimum</source>
            <translation>原子最小值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="175" />
            <source>Atomic maximum</source>
            <translation>原子最大值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="176" />
            <source>Atomic bitwise AND</source>
            <translation>原子按位与</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="177" />
            <source>Atomic bitwise OR</source>
            <translation>原子按位或</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="178" />
            <source>Atomic bitwise XOR</source>
            <translation>原子按位异或</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="179" />
            <source>Atomic exchange</source>
            <translation>原子交换</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="180" />
            <source>Atomic compare and swap</source>
            <translation>原子比较并交换</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="183" />
            <source>Synchronizes every invocation in the work group</source>
            <translation>同步当前工作组内所有调用</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="184" />
            <source>Memory barrier for all memory types</source>
            <translation>所有内存类型的内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="185" />
            <source>Memory barrier for shared memory</source>
            <translation>共享内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="186" />
            <source>Memory barrier for buffer memory</source>
            <translation>缓冲区内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="187" />
            <source>Memory barrier for image memory</source>
            <translation>图像内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="188" />
            <source>Memory barrier for atomic counters</source>
            <translation>原子计数器内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="189" />
            <source>Work-group memory barrier</source>
            <translation>工作组内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="192" />
            <source>Partial derivative in x (precision is implementation-defined)</source>
            <translation>x 方向偏导数（精度由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="193" />
            <source>Partial derivative in y (precision is implementation-defined)</source>
            <translation>y 方向偏导数（精度由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="194" />
            <source>Partial derivative in x (fine)</source>
            <translation>x 方向偏导数（精细）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="195" />
            <source>Partial derivative in y (fine)</source>
            <translation>y 方向偏导数（精细）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="196" />
            <source>Partial derivative in x (coarse)</source>
            <translation>x 方向偏导数（粗糙）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="197" />
            <source>Partial derivative in y (coarse)</source>
            <translation>y 方向偏导数（粗糙）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="198" />
            <source>abs(dFdx) + abs(dFdy)</source>
            <translation>abs(dFdx) + abs(dFdy)</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="199" />
            <source>Fine-precision fwidth</source>
            <translation>精细精度 fwidth</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="200" />
            <source>Coarse-precision fwidth</source>
            <translation>粗糙精度 fwidth</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="203" />
            <source>Interpolates at the centroid</source>
            <translation>在质心处插值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="204" />
            <source>Interpolates at the given sample</source>
            <translation>在给定采样处插值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="205" />
            <source>Interpolates at the given offset</source>
            <translation>在给定偏移处插值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="208" />
            <source>Sum across the subgroup</source>
            <translation>子组内求和</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="209" />
            <source>Product across the subgroup</source>
            <translation>子组内求积</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="210" />
            <source>Minimum across the subgroup</source>
            <translation>子组内最小值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="211" />
            <source>Maximum across the subgroup</source>
            <translation>子组内最大值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="212" />
            <source>Bitwise AND across the subgroup</source>
            <translation>子组内按位与</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="213" />
            <source>Bitwise OR across the subgroup</source>
            <translation>子组内按位或</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="214" />
            <source>Bitwise XOR across the subgroup</source>
            <translation>子组内按位异或</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="215" />
            <source>True when the value is true in every lane of the subgroup</source>
            <translation>子组内每个 lane 都为真时为真</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="216" />
            <source>True when the value is true in any lane of the subgroup</source>
            <translation>子组内任一 lane 为真时为真</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="217" />
            <source>Subgroup vote returning a bit mask</source>
            <translation>返回位掩码的子组投票</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="218" />
            <source>Broadcasts the value of the given lane</source>
            <translation>广播指定 lane 的值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="219" />
            <source>Broadcasts the value of the lowest lane</source>
            <translation>广播最低 lane 的值</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="220" />
            <source>Elects a representative lane of the subgroup</source>
            <translation>选出子组的代表 lane</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="221" />
            <source>Synchronizes within the subgroup</source>
            <translation>子组内同步</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="222" />
            <source>Subgroup memory barrier</source>
            <translation>子组内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="223" />
            <source>Subgroup memory barrier for buffer memory</source>
            <translation>缓冲区内存的子组内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="224" />
            <source>Subgroup memory barrier for shared memory</source>
            <translation>共享内存的子组内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="225" />
            <source>Subgroup memory barrier for image memory</source>
            <translation>图像内存的子组内存屏障</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="226" />
            <source>Counts the set bits in a bit mask</source>
            <translation>统计位掩码中置位的比特数</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="229" />
            <source>1D noise (implementation-defined)</source>
            <translation>1D 噪声（由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="230" />
            <source>2D noise (implementation-defined)</source>
            <translation>2D 噪声（由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="231" />
            <source>3D noise (implementation-defined)</source>
            <translation>3D 噪声（由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="232" />
            <source>4D noise (implementation-defined)</source>
            <translation>4D 噪声（由实现定义）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="533" />
            <source>Scalar type</source>
            <translation>标量类型</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="534" />
            <source>Vector type</source>
            <translation>向量类型</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="535" />
            <source>Matrix type</source>
            <translation>矩阵类型</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="536" />
            <source>Opaque type (sampler / image)</source>
            <translation>不透明类型（sampler / image）</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="556" />
            <source>Qualifier / storage class</source>
            <translation>限定符 / 存储类</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="557" />
            <source>Control-flow keyword</source>
            <translation>控制流关键字</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="558" />
            <source>Built-in variable</source>
            <translation>内建变量</translation>
        </message>
        <message>
            <location filename="../ui/GlslLanguageData.cpp" line="562" />
            <source>Preprocessor directive</source>
            <translation>预处理指令</translation>
        </message>
    </context>
    <context>
        <name>Preprocessor</name>
        <message>
            <location filename="../core/IncludePreprocessor.cpp" line="78" />
            <source>Cannot read file %1: %2</source>
            <translation>无法读取文件 %1：%2</translation>
        </message>
        <message>
            <location filename="../core/IncludePreprocessor.cpp" line="185" />
            <source>Cannot find the file "%1" for this #include. Add an #include search path in the settings, or use a path relative to the current file.</source>
            <translation>找不到此 #include 对应的文件“%1”。请在设置中添加 #include 搜索路径，或使用相对于当前文件的路径。</translation>
        </message>
        <message>
            <location filename="../core/IncludePreprocessor.cpp" line="199" />
            <source>#include nesting is deeper than %1 levels. Check for an overly deep include chain.</source>
            <translation>#include 嵌套深度超过 %1 层。请检查是否存在过深的包含链。</translation>
        </message>
        <message>
            <location filename="../core/IncludePreprocessor.cpp" line="209" />
            <source>Circular #include detected: %1 has already been included. Break this include cycle.</source>
            <translation>检测到循环 #include：%1 已被包含。请消除这一循环。</translation>
        </message>
    </context>
    <context>
        <name>Reflection</name>
        <message>
            <location filename="../core/ReflectData.cpp" line="160" />
            <source>
(this module exposes no descriptor, push constant or IO variable)</source>
            <translation>（此模块未暴露任何描述符、推送常量或 IO 变量）</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="161" />
            <source>Unknown (%1)</source>
            <translation>未知（%1）</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="228" />
            <source>Model %1</source>
            <translation>模型 %1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="255" />
            <source>Generator %1</source>
            <translation>生成器 %1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="310" />
            <source>Success</source>
            <translation>成功</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="312" />
            <source>The module is not ready yet</source>
            <translation>模块尚未就绪</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="314" />
            <source>SPIR-V parsing failed (the module may be corrupt)</source>
            <translation>SPIR-V 解析失败（模块可能已损坏）</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="316" />
            <source>Memory allocation failed</source>
            <translation>内存分配失败</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="318" />
            <source>Index out of range</source>
            <translation>索引超出范围</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="320" />
            <source>A null pointer was passed in</source>
            <translation>传入了空指针</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="322" />
            <source>SPIRV-Reflect internal error</source>
            <translation>SPIRV-Reflect 内部错误</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="324" />
            <source>Enumeration count mismatch (the module changed between calls)</source>
            <translation>枚举计数不匹配（模块在两次调用之间发生了变化）</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="326" />
            <source>The requested element was not found</source>
            <translation>未找到请求的元素</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="328" />
            <source>The SPIR-V byte count is not a multiple of 4</source>
            <translation>SPIR-V 字节数不是 4 的倍数</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="330" />
            <source>Invalid SPIR-V magic number</source>
            <translation>SPIR-V 魔数无效</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="332" />
            <source>The SPIR-V data ends unexpectedly</source>
            <translation>SPIR-V 数据意外结束</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="334" />
            <source>The SPIR-V references an invalid id</source>
            <translation>SPIR-V 引用了无效的 id</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="336" />
            <source>Descriptor set number overflow</source>
            <translation>描述符 set 编号溢出</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="338" />
            <source>Invalid SPIR-V storage class</source>
            <translation>SPIR-V 存储类无效</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="340" />
            <source>Recursion while parsing the SPIR-V</source>
            <translation>解析 SPIR-V 时发生递归</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="342" />
            <source>Invalid SPIR-V instruction</source>
            <translation>SPIR-V 指令无效</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="344" />
            <source>Unexpected SPIR-V block data</source>
            <translation>SPIR-V 块数据异常</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="346" />
            <source>Invalid SPIR-V block member reference</source>
            <translation>SPIR-V 块成员引用无效</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="348" />
            <source>Invalid SPIR-V entry point</source>
            <translation>SPIR-V 入口点无效</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="350" />
            <source>Invalid SPIR-V execution mode</source>
            <translation>SPIR-V 执行模式无效</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="352" />
            <source>SPIR-V recursion depth exceeded</source>
            <translation>SPIR-V 递归深度超限</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="354" />
            <source>Unknown error (%1)</source>
            <translation>未知错误（%1）</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="369" />
            <source>The SPIR-V data is shorter than 5 words, so it cannot be reflected.</source>
            <translation>SPIR-V 数据不足 5 个字，无法进行反射。</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="375" />
            <source>The SPIR-V magic number is not 0x07230203, so it cannot be reflected.</source>
            <translation>SPIR-V 魔数不是 0x07230203，无法进行反射。</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="384" />
            <source>Reflection failed: %1. If the shader was just edited, recompile it before looking at the reflection information again.</source>
            <translation>反射失败：%1。如果着色器刚刚被修改，请先重新编译，再查看反射信息。</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="422" />
            <source>Mode %1</source>
            <translation>模式 %1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="440" />
            <source>Failed to enumerate the descriptor binding count: %1</source>
            <translation>枚举描述符 binding 数量失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="448" />
            <source>Failed to read the descriptor bindings: %1</source>
            <translation>读取描述符 binding 失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="487" />
            <source>Failed to enumerate the push constant blocks: %1</source>
            <translation>枚举推送常量块失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="495" />
            <source>Failed to read the push constant blocks: %1</source>
            <translation>读取推送常量块失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="595" />
            <source>Failed to enumerate the input variables: %1</source>
            <translation>枚举输入变量失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="602" />
            <source>Failed to enumerate the output variables: %1</source>
            <translation>枚举输出变量失败：%1</translation>
        </message>
        <message>
            <location filename="../core/SpirvReflection.cpp" line="629" />
            <source>(not provided)</source>
            <translation>（未提供）</translation>
        </message>
    </context>
    <context>
        <name>main</name>
        <message>
            <location filename="../app/main.cpp" line="29" />
            <source>Startup Failed</source>
            <translation>启动失败</translation>
        </message>
        <message>
            <location filename="../app/main.cpp" line="30" />
            <source>glslang could not be initialized, so the compiler is unavailable.

Restart the application; if the problem persists, check that the installation is complete (glslang static libraries).</source>
            <translation>无法初始化 glslang，编译器不可用。\n\n请重启程序；若问题依旧，请检查安装是否完整（glslang 静态库）。</translation>
        </message>
    </context>
    <context>
        <name>spall::AsmPanel</name>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="132" />
            <location filename="../ui/panels/AsmPanel.cpp" line="197" />
            <source>Indent</source>
            <translation>缩进</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="134" />
            <location filename="../ui/panels/AsmPanel.cpp" line="198" />
            <source>Friendly names</source>
            <translation>友好名称</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="136" />
            <location filename="../ui/panels/AsmPanel.cpp" line="199" />
            <source>Comments</source>
            <translation>注释</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="138" />
            <location filename="../ui/panels/AsmPanel.cpp" line="200" />
            <source>Byte offsets</source>
            <translation>字节偏移</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="140" />
            <location filename="../ui/panels/AsmPanel.cpp" line="201" />
            <source>Nested indent</source>
            <translation>嵌套缩进</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="142" />
            <location filename="../ui/panels/AsmPanel.cpp" line="202" />
            <source>Reorder blocks</source>
            <translation>重排基本块</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="144" />
            <location filename="../ui/panels/AsmPanel.cpp" line="203" />
            <source>No header</source>
            <translation>无头</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="151" />
            <location filename="../ui/panels/AsmPanel.cpp" line="205" />
            <source>Save .spv</source>
            <translation>保存 .spv</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="153" />
            <location filename="../ui/panels/AsmPanel.cpp" line="207" />
            <source>Save the compiled SPIR-V binary as a .spv file that spirv-dis / spirv-val or a Vulkan driver can consume directly</source>
            <translation>将编译好的 SPIR-V 二进制保存为 .spv 文件，spirv-dis / spirv-val 或 Vulkan 驱动可直接使用</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="155" />
            <location filename="../ui/panels/AsmPanel.cpp" line="209" />
            <source>Export .spvasm</source>
            <translation>导出 .spvasm</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="156" />
            <location filename="../ui/panels/AsmPanel.cpp" line="210" />
            <source>Save the disassembly text as a .spvasm file</source>
            <translation>将反汇编文本保存为 .spvasm 文件</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="157" />
            <location filename="../ui/panels/AsmPanel.cpp" line="211" />
            <source>Copy All</source>
            <translation>全部复制</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="158" />
            <location filename="../ui/panels/AsmPanel.cpp" line="212" />
            <location filename="../ui/panels/AsmPanel.cpp" line="354" />
            <location filename="../ui/panels/AsmPanel.cpp" line="365" />
            <source>Find</source>
            <translation>查找</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="262" />
            <source>&lt;b&gt;words&lt;/b&gt; %1 &amp;nbsp; &lt;b&gt;instructions&lt;/b&gt; %2 &amp;nbsp; &lt;b&gt;id bound&lt;/b&gt; %3 &amp;nbsp; &lt;b&gt;SPIR-V&lt;/b&gt; %4</source>
            <translation>&lt;b&gt;word&lt;/b&gt; %1 &amp;nbsp; &lt;b&gt;指令&lt;/b&gt; %2 &amp;nbsp; &lt;b&gt;id 上界&lt;/b&gt; %3 &amp;nbsp; &lt;b&gt;SPIR-V&lt;/b&gt; %4</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="273" />
            <source>Nothing compiled yet. Press F7 to compile the current tab.</source>
            <translation>尚未编译。按 F7 编译当前标签页。</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="275" />
            <source>Stale, press F7 to recompile</source>
            <translation>已过期，按 F7 重新编译</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="277" />
            <source>(the disassembly is empty)</source>
            <translation>（反汇编为空）</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="324" />
            <source>; Large module (%1 lines), loading in chunks...
</source>
            <translation>；大模块（%1 行），正在分块加载...\n</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="354" />
            <source>Find in assembly:</source>
            <translation>在汇编中查找：</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="365" />
            <source>Cannot find "%1".</source>
            <translation>找不到“%1”。</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="384" />
            <source>Save SPIR-V</source>
            <translation>保存 SPIR-V</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="385" />
            <source>There is no SPIR-V module to save.

Compile successfully with F7 first.</source>
            <translation>没有可保存的 SPIR-V 模块。\n\n请先用 F7 成功编译。</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="390" />
            <source>Save SPIR-V Binary</source>
            <translation>保存 SPIR-V 二进制</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="391" />
            <source>SPIR-V binary (*.spv);;All files (*)</source>
            <translation>SPIR-V 二进制 (*.spv);;所有文件 (*)</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="407" />
            <location filename="../ui/panels/AsmPanel.cpp" line="416" />
            <source>Save Failed</source>
            <translation>保存失败</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="408" />
            <location filename="../ui/panels/AsmPanel.cpp" line="450" />
            <source>Cannot write %1: %2

Choose another directory, or check whether the file is in use.</source>
            <translation>无法写入 %1：%2\n\n请另选目录，或检查文件是否被占用。</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="417" />
            <source>Only %1 of %2 bytes were written; the disk may be full.</source>
            <translation>只写入了 %2 字节中的 %1 字节；磁盘可能已满。</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="424" />
            <source>Save Succeeded</source>
            <translation>保存成功</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="425" />
            <source>Saved %1 words (%2 bytes) to
%3

Verify with:
  spirv-val "%3"
  spirv-dis "%3"</source>
            <translation>已保存 %1 word（%2 字节）到\n%3\n\n可用以下命令校验：\n  spirv-val "%3"\n  spirv-dis "%3"</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="436" />
            <source>Export Assembly</source>
            <translation>导出汇编</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="437" />
            <source>There is no assembly text to export. Press F7 to compile first.</source>
            <translation>没有可导出的汇编文本。请先按 F7 编译。</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="442" />
            <source>Export SPIR-V Assembly</source>
            <translation>导出 SPIR-V 汇编</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="443" />
            <source>SPIR-V assembly (*.spvasm);;All files (*)</source>
            <translation>SPIR-V 汇编 (*.spvasm);;所有文件 (*)</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="449" />
            <source>Export Failed</source>
            <translation>导出失败</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="500" />
            <source>Go to Label</source>
            <translation>跳转到 OpLabel</translation>
        </message>
        <message>
            <location filename="../ui/panels/AsmPanel.cpp" line="501" />
            <source>No definition found for label %1 in the assembly.</source>
            <translation>在汇编中找不到 %1 的定义。</translation>
        </message>
    </context>
    <context>
        <name>spall::DiagnosticsPanel</name>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="20" />
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="44" />
            <source>Show warnings and hints</source>
            <translation>显示警告和提示</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="28" />
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="45" />
            <source>Severity</source>
            <translation>严重级</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="28" />
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="45" />
            <source>Category</source>
            <translation>类别</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="29" />
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="46" />
            <source>Location</source>
            <translation>位置</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="29" />
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="46" />
            <source>Message</source>
            <translation>消息</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="54" />
            <source>✖ Error</source>
            <translation>✖ 错误</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="55" />
            <source>⚠ Warning</source>
            <translation>⚠ 警告</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="56" />
            <source>ℹ Info</source>
            <translation>ℹ 信息</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="57" />
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="59" />
            <source>· Hint</source>
            <translation>· 提示</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="88" />
            <source>&lt;editor&gt;</source>
            <translation>&lt;编辑器&gt;</translation>
        </message>
        <message>
            <location filename="../ui/panels/DiagnosticsPanel.cpp" line="94" />
            <source>Global</source>
            <translation>全局</translation>
        </message>
    </context>
    <context>
        <name>spall::FileBrowser</name>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="124" />
            <source>Folder currently being browsed</source>
            <translation>当前浏览的文件夹</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="125" />
            <location filename="../ui/FileBrowser.cpp" line="170" />
            <source>Choose the folder to browse</source>
            <translation>选择要浏览的文件夹</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="126" />
            <source>Filter by name...</source>
            <translation>按名称过滤…</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="190" />
            <source>Cannot Open</source>
            <translation>无法打开</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="191" />
            <source>%1 is not a shader file this tool can compile.

Supported extensions: %2</source>
            <translation>%1 不是本工具可编译的着色器文件。\n\n支持的扩展名：%2</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="209" />
            <source>Browse This Folder</source>
            <translation>浏览此文件夹</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="212" />
            <source>Open</source>
            <translation>打开</translation>
        </message>
        <message>
            <location filename="../ui/FileBrowser.cpp" line="216" />
            <source>Reveal in Folder</source>
            <translation>定位到所在文件夹</translation>
        </message>
    </context>
    <context>
        <name>spall::GlslCompleter</name>
        <message>
            <location filename="../ui/GlslCompleter.cpp" line="451" />
            <source>Symbol from the compiled module</source>
            <translation>来自已编译模块的符号</translation>
        </message>
        <message>
            <location filename="../ui/GlslCompleter.cpp" line="458" />
            <source>Identifier declared in this document</source>
            <translation>本文档中声明的标识符</translation>
        </message>
        <message>
            <location filename="../ui/GlslCompleter.cpp" line="469" />
            <source>Function defined in this document</source>
            <translation>本文档中定义的函数</translation>
        </message>
    </context>
    <context>
        <name>spall::MainWindow</name>
        <message>
            <location filename="../ui/MainWindow.cpp" line="133" />
            <location filename="../ui/MainWindow.cpp" line="657" />
            <source>Ready</source>
            <translation>就绪</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="143" />
            <location filename="../ui/MainWindow.cpp" line="605" />
            <location filename="../ui/MainWindow.cpp" line="1260" />
            <source>Compile</source>
            <translation>编译</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="146" />
            <location filename="../ui/MainWindow.cpp" line="606" />
            <source>Compile the current tab (F7 / Ctrl+B)</source>
            <translation>编译当前标签页（F7 / Ctrl+B）</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="156" />
            <location filename="../ui/MainWindow.cpp" line="607" />
            <source>Compile All</source>
            <translation>全部编译</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="162" />
            <location filename="../ui/MainWindow.cpp" line="608" />
            <location filename="../ui/MainWindow.cpp" line="1467" />
            <location filename="../ui/MainWindow.cpp" line="1475" />
            <source>Validate</source>
            <translation>校验</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="163" />
            <location filename="../ui/MainWindow.cpp" line="609" />
            <source>Re-run SPIR-V validation on the current module</source>
            <translation>对当前模块重新执行 SPIR-V 校验</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="166" />
            <location filename="../ui/MainWindow.cpp" line="610" />
            <location filename="../ui/MainWindow.cpp" line="1493" />
            <source>Disassemble</source>
            <translation>反汇编</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="167" />
            <location filename="../ui/MainWindow.cpp" line="611" />
            <source>Disassemble again with the current options</source>
            <translation>使用当前选项再次反汇编</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="170" />
            <location filename="../ui/MainWindow.cpp" line="602" />
            <source>Save</source>
            <translation>保存</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="174" />
            <location filename="../ui/MainWindow.cpp" line="603" />
            <source>Save As...</source>
            <translation>另存为…</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="178" />
            <location filename="../ui/MainWindow.cpp" line="604" />
            <source>Reload from Disk</source>
            <translation>从磁盘重新加载</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="181" />
            <location filename="../ui/MainWindow.cpp" line="600" />
            <source>New</source>
            <translation>新建</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="185" />
            <location filename="../ui/MainWindow.cpp" line="601" />
            <source>Open...</source>
            <translation>打开…</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="201" />
            <location filename="../ui/MainWindow.cpp" line="612" />
            <source>Export Assembly...</source>
            <translation>导出汇编…</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="207" />
            <location filename="../ui/MainWindow.cpp" line="613" />
            <source>Copy Assembly</source>
            <translation>复制汇编</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="215" />
            <location filename="../ui/MainWindow.cpp" line="614" />
            <source>Save SPIR-V (.spv)</source>
            <translation>保存 SPIR-V (.spv)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="218" />
            <location filename="../ui/MainWindow.cpp" line="615" />
            <source>Write the compiled SPIR-V binary to a .spv file (Ctrl+Shift+E)</source>
            <translation>将编译得到的 SPIR-V 二进制写入 .spv 文件（Ctrl+Shift+E）</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="223" />
            <location filename="../ui/MainWindow.cpp" line="617" />
            <source>Settings...</source>
            <translation>设置…</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="226" />
            <location filename="../ui/MainWindow.cpp" line="618" />
            <location filename="../ui/MainWindow.cpp" line="941" />
            <source>About SpAll</source>
            <translation>关于 SpAll</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="244" />
            <location filename="../ui/MainWindow.cpp" line="622" />
            <source>Main Toolbar</source>
            <translation>主工具栏</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="251" />
            <location filename="../ui/MainWindow.cpp" line="623" />
            <source> Stage </source>
            <translation> 阶段 </translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="255" />
            <location filename="../ui/MainWindow.cpp" line="624" />
            <source>Shader stage of the current tab</source>
            <translation>当前标签页的着色器阶段</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="259" />
            <location filename="../ui/MainWindow.cpp" line="625" />
            <source> Version </source>
            <translation> 版本 </translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="265" />
            <location filename="../ui/MainWindow.cpp" line="626" />
            <source>GLSL #version</source>
            <translation>GLSL #version</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="269" />
            <location filename="../ui/MainWindow.cpp" line="627" />
            <source> Vulkan </source>
            <translation> Vulkan </translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="276" />
            <location filename="../ui/MainWindow.cpp" line="628" />
            <source>Target Vulkan version (decides the SPIR-V version and the validation environment)</source>
            <translation>目标 Vulkan 版本（决定 SPIR-V 版本与校验环境）</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="290" />
            <location filename="../ui/MainWindow.cpp" line="630" />
            <source> Optimize </source>
            <translation> 优化 </translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="297" />
            <location filename="../ui/MainWindow.cpp" line="631" />
            <source>SPIRV-Tools optimization level</source>
            <translation>SPIRV-Tools 优化级别</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="310" />
            <location filename="../ui/MainWindow.cpp" line="636" />
            <location filename="../ui/MainWindow.cpp" line="646" />
            <source>Files</source>
            <translation>文件</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="329" />
            <location filename="../ui/MainWindow.cpp" line="637" />
            <source>Click a file name to open it</source>
            <translation>点击文件名即可打开</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="343" />
            <location filename="../ui/MainWindow.cpp" line="638" />
            <location filename="../ui/MainWindow.cpp" line="647" />
            <source>Output</source>
            <translation>输出</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="350" />
            <location filename="../ui/MainWindow.cpp" line="639" />
            <source>SPIR-V Assembly</source>
            <translation>SPIR-V 汇编</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="351" />
            <location filename="../ui/MainWindow.cpp" line="640" />
            <source>Reflection</source>
            <translation>反射</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="352" />
            <location filename="../ui/MainWindow.cpp" line="641" />
            <source>Stats / Log</source>
            <translation>统计 / 日志</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="364" />
            <location filename="../ui/MainWindow.cpp" line="642" />
            <location filename="../ui/MainWindow.cpp" line="648" />
            <source>Problems</source>
            <translation>问题</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="448" />
            <source>Unsaved Changes</source>
            <translation>未保存的修改</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="449" />
            <source>%1 has unsaved changes.

Do you want to save them?</source>
            <translation>%1 有未保存的修改。\n\n是否保存？</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="555" />
            <location filename="../ui/MainWindow.cpp" line="595" />
            <source>Language</source>
            <translation>语言</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="556" />
            <source>The translation catalogue for "%1" could not be loaded.

Expected a file named spall_%2.qm next to the executable, in a translations/ sub-directory. The interface stays in the current language.</source>
            <translation>无法加载“%1”的翻译文件。\n\n预期在可执行文件同级的 translations/ 子目录下存在名为 spall_%2.qm 的文件。界面将保持当前语言。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="591" />
            <source>&amp;File</source>
            <translation>文件(&amp;F)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="592" />
            <source>Recent Files</source>
            <translation>最近文件</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="593" />
            <source>&amp;Build</source>
            <translation>构建(&amp;B)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="594" />
            <source>&amp;View</source>
            <translation>视图(&amp;V)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="596" />
            <source>&amp;Help</source>
            <translation>帮助(&amp;H)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="692" />
            <source>(none)</source>
            <translation>（无）</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="725" />
            <source>Unsaved new file</source>
            <translation>未保存的新建文件</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="942" />
            <source>&lt;b&gt;SpAll %1&lt;/b&gt;&lt;br&gt;&lt;br&gt;A lightweight GLSL editor with SPIR-V compile/disassemble tools.&lt;br&gt;glslang and SPIRV-Tools/SPIRV-Reflect are compiled into this process; no external programs are invoked.&lt;br&gt;&lt;br&gt;F7 compile · Ctrl+Space completion · F8 next problem · Ctrl+1..4 switch right tabs</source>
            <translation>&lt;b&gt;SpAll %1&lt;/b&gt;&lt;br&gt;&lt;br&gt;轻量级 GLSL 编辑器，内置 SPIR-V 编译/反汇编工具。&lt;br&gt;glslang 与 SPIRV-Tools/SPIRV-Reflect 已编译进本进程，不调用任何外部程序。&lt;br&gt;&lt;br&gt;F7 编译 · Ctrl+Space 补全 · F8 下一个问题 · Ctrl+1..4 切换右侧标签页</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="963" />
            <location filename="../ui/MainWindow.cpp" line="1625" />
            <source>Line %1, Column %2</source>
            <translation>第 %1 行，第 %2 列</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="981" />
            <source>Open Shader</source>
            <translation>打开着色器</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1008" />
            <location filename="../ui/MainWindow.cpp" line="1046" />
            <source>Open Failed</source>
            <translation>打开失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1009" />
            <source>Could not read %1: %2

Check the file permissions and make sure the file is not locked by another program.</source>
            <translation>无法读取 %1：%2\n\n请检查文件权限，并确认文件未被其他程序锁定。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1036" />
            <source>Opened %1</source>
            <translation>已打开 %1</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1047" />
            <source>The file does not exist: %1

It may have been moved or deleted, so the entry has been removed from the recent files list.</source>
            <translation>文件不存在：%1\n\n该文件可能已被移动或删除，因此已从最近文件列表中移除。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1069" />
            <source>Save Failed</source>
            <translation>保存失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1070" />
            <source>Could not write %1: %2

Check whether the file is read-only or locked by another program.</source>
            <translation>无法写入 %1：%2\n\n请检查文件是否为只读，或被其他程序锁定。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1078" />
            <location filename="../ui/MainWindow.cpp" line="1109" />
            <source>Saved</source>
            <translation>已保存</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1090" />
            <source>Save As</source>
            <translation>另存为</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1091" />
            <source>Shaders (*);;All Files (*)</source>
            <translation>着色器 (*);;所有文件 (*)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1097" />
            <source>Save As Failed</source>
            <translation>另存为失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1098" />
            <source>Could not write %1: %2</source>
            <translation>无法写入 %1：%2</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1119" />
            <source>Reload</source>
            <translation>重新加载</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1120" />
            <source>Reloading discards the current unsaved changes. Continue?</source>
            <translation>重新加载将丢弃当前未保存的修改。是否继续？</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1129" />
            <source>Reload Failed</source>
            <translation>重新加载失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1130" />
            <source>Could not read %1: %2</source>
            <translation>无法读取 %1：%2</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1140" />
            <source>Reloaded</source>
            <translation>已重新加载</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1189" />
            <source>Compiling...</source>
            <translation>正在编译…</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1261" />
            <source>The current tab is empty.

Type some GLSL code first, or open one of the examples from examples/ on the left.</source>
            <translation>当前标签页为空。\n\n请先输入一些 GLSL 代码，或从左侧的 examples/ 中打开一个示例。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1325" />
            <source>Compile succeeded</source>
            <translation>编译成功</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1326" />
            <source>Compile failed</source>
            <translation>编译失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1373" />
            <source>=== Compile %1 ===</source>
            <translation>=== 编译 %1 ===</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1373" />
            <source>&lt;untitled&gt;</source>
            <translation>&lt;未命名&gt;</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1374" />
            <source>Elapsed %1 ms</source>
            <translation>耗时 %1 ms</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1375" />
            <source>Success: %1</source>
            <translation>成功：%1</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1375" />
            <source>yes</source>
            <translation>是</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1375" />
            <source>no</source>
            <translation>否</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1376" />
            <source>Errors %1 / Warnings %2 / Hints %3</source>
            <translation>错误 %1 / 警告 %2 / 提示 %3</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1387" />
            <source>
--- Diagnostics ---</source>
            <translation>\n--- 诊断 ---</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1407" />
            <source>Compile %1 ms</source>
            <translation>编译 %1 ms</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1408" />
            <source>words %1</source>
            <translation>词数 %1</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1411" />
            <source>Optimized %1</source>
            <translation>优化后 %1</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1422" />
            <source>%1 (%2 errors)</source>
            <translation>%1（%2 个错误）</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1468" />
            <source>There is no module to validate yet.

Compile successfully with F7 first.</source>
            <translation>尚无模块可校验。\n\n请先用 F7 编译成功。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1476" />
            <source>SPIR-V validation passed (environment: %1).</source>
            <translation>SPIR-V 校验通过（环境：%1）。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1484" />
            <source>Validation Failed</source>
            <translation>校验失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1485" />
            <source>SPIR-V validation failed:

%1

The usual cause is a missing layout(binding=) with automatic assignment turned off.</source>
            <translation>SPIR-V 校验失败：\n\n%1\n\n常见原因是关闭自动分配后缺少 layout(binding=)。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1494" />
            <source>There is no module to disassemble yet.

Compile successfully with F7 first.</source>
            <translation>尚无模块可反汇编。\n\n请先用 F7 编译成功。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1506" />
            <source>Disassembly Failed</source>
            <translation>反汇编失败</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1569" />
            <location filename="../ui/MainWindow.cpp" line="1578" />
            <source>No problems</source>
            <translation>没有问题</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1716" />
            <source>Assembly copied to the clipboard</source>
            <translation>汇编已复制到剪贴板</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1739" />
            <source>(examples/ directory not found)</source>
            <translation>（未找到 examples/ 目录）</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1804" />
            <source>Missing #version directive. #version must be the first effective statement in the file; write "#version 450".</source>
            <translation>缺少 #version 指令。#version 必须是文件中第一条有效语句；请写 "#version 450"。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1838" />
            <source>Unbalanced closing bracket: this '}' or ')' has no matching opener.</source>
            <translation>括号不匹配：该 '}' 或 ')' 没有对应的开括号。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1841" />
            <source>Unbalanced braces: %1 more '}' expected.</source>
            <translation>花括号不匹配：还缺少 %1 个 '}'。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1845" />
            <source>Unbalanced parentheses: %1 more ')' expected.</source>
            <translation>圆括号不匹配：还缺少 %1 个 ')'。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1870" />
            <source>GLSL has no vec4f; use vec4</source>
            <translation>GLSL 没有 vec4f；请使用 vec4</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1871" />
            <source>GLSL has no vec3f; use vec3</source>
            <translation>GLSL 没有 vec3f；请使用 vec3</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1872" />
            <source>GLSL has no vec2f; use vec2</source>
            <translation>GLSL 没有 vec2f；请使用 vec2</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1873" />
            <source>float4 is HLSL; use vec4 in GLSL</source>
            <translation>float4 属于 HLSL；GLSL 中请使用 vec4</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1874" />
            <source>float3 is HLSL; use vec3 in GLSL</source>
            <translation>float3 属于 HLSL；GLSL 中请使用 vec3</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1875" />
            <source>float2 is HLSL; use vec2 in GLSL</source>
            <translation>float2 属于 HLSL；GLSL 中请使用 vec2</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1876" />
            <source>mat3x3 should be written mat3</source>
            <translation>mat3x3 应写作 mat3</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1877" />
            <source>mat4x4 should be written mat4</source>
            <translation>mat4x4 应写作 mat4</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1878" />
            <source>mat2x2 should be written mat2</source>
            <translation>mat2x2 应写作 mat2</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1879" />
            <source>sampler2d is the wrong case; use sampler2D</source>
            <translation>sampler2d 大小写错误；请使用 sampler2D</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1880" />
            <source>Sampler2D is the wrong case; use sampler2D</source>
            <translation>Sampler2D 大小写错误；请使用 sampler2D</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1882" />
            <source>Vulkan GLSL: use texture(...), not texture2D(...)</source>
            <translation>Vulkan GLSL：请使用 texture(...)，而不是 texture2D(...)</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1884" />
            <source>gl_FragColor is compatibility-mode only; declare an out variable in Vulkan</source>
            <translation>gl_FragColor 仅适用于兼容模式；在 Vulkan 中请声明 out 变量</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1902" />
            <source>Under a Vulkan target the profile keyword core is redundant; write "#version 450" instead.</source>
            <translation>在 Vulkan 目标下，profile 关键字 core 是多余的；请改写为 "#version 450"。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1911" />
            <source>No main() function found. A shader must define void main().</source>
            <translation>未找到 main() 函数。着色器必须定义 void main()。</translation>
        </message>
        <message>
            <location filename="../ui/MainWindow.cpp" line="1915" />
            <source>The vertex shader never assigns gl_Position, so rasterization has no valid position.</source>
            <translation>顶点着色器从未给 gl_Position 赋值，光栅化将没有有效位置。</translation>
        </message>
    </context>
    <context>
        <name>spall::OutputPanel</name>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="34" />
            <location filename="../ui/panels/OutputPanel.cpp" line="63" />
            <source>Item</source>
            <translation>项</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="34" />
            <location filename="../ui/panels/OutputPanel.cpp" line="63" />
            <source>Value</source>
            <translation>值</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="39" />
            <location filename="../ui/panels/OutputPanel.cpp" line="60" />
            <source>Stats</source>
            <translation>统计</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="45" />
            <location filename="../ui/panels/OutputPanel.cpp" line="64" />
            <source>Compiler / validator / optimizer messages appear here.</source>
            <translation>编译器 / 校验器 / 优化器消息显示在这里。</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="46" />
            <location filename="../ui/panels/OutputPanel.cpp" line="61" />
            <source>Log</source>
            <translation>日志</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="53" />
            <location filename="../ui/panels/OutputPanel.cpp" line="65" />
            <source>Hex dump of the optimized SPIR-V module.</source>
            <translation>优化后 SPIR-V 模块的十六进制转储。</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="54" />
            <location filename="../ui/panels/OutputPanel.cpp" line="62" />
            <source>Binary</source>
            <translation>二进制</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="89" />
            <source>Compile time</source>
            <translation>编译耗时</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="90" />
            <source>SPIR-V version</source>
            <translation>SPIR-V 版本</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="91" />
            <source>Optimization level</source>
            <translation>优化级别</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="92" />
            <source>Words (before optimization)</source>
            <translation>word 数（优化前）</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="93" />
            <source>Words (after optimization)</source>
            <translation>word 数（优化后）</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="94" />
            <source>Instructions (before optimization)</source>
            <translation>指令数（优化前）</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="96" />
            <source>Instructions (after optimization)</source>
            <translation>指令数（优化后）</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="97" />
            <source>id bound</source>
            <translation>id 上界</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="102" />
            <source>Word delta</source>
            <translation>word 增减</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="122" />
            <source>Top %1 opcodes by frequency</source>
            <translation>出现频率最高的 %1 个 OpCode</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="123" />
            <source>opcode kinds: %1</source>
            <translation>OpCode 种类：%1</translation>
        </message>
        <message>
            <location filename="../ui/panels/OutputPanel.cpp" line="146" />
            <source>; words: %1, bytes: %2</source>
            <translation>word：%1，字节：%2</translation>
        </message>
    </context>
    <context>
        <name>spall::ReflectPanel</name>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="94" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="132" />
            <source>Hide unused variables</source>
            <translation>隐藏未使用的变量</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="96" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="134" />
            <source>Bindings removed by the optimizer are shown in grey; tick this to hide them.</source>
            <translation>被优化器移除的 binding 以灰色显示；勾选此项可将其隐藏。</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="102" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="135" />
            <source>Item</source>
            <translation>项目</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="102" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="135" />
            <source>Value</source>
            <translation>值</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="111" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="136" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="333" />
            <source>Copy as Text</source>
            <translation>复制为文本</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="112" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="137" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="401" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="426" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="435" />
            <source>Generate C++ Struct</source>
            <translation>生成 C++ struct</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="114" />
            <location filename="../ui/panels/ReflectPanel.cpp" line="139" />
            <source>Generate a std140-aligned struct from the reflected offsets/padding, with a sizeof assertion.</source>
            <translation>根据反射得到的 offset/padding 生成 std140 对齐的 struct，并附带 sizeof 断言。</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="163" />
            <source>No reflection data yet. It is generated after a successful compile.</source>
            <translation>暂无反射数据，编译成功后才会生成。</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="166" />
            <source>(not compiled yet)</source>
            <translation>（尚未编译）</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="167" />
            <source>Press F7 to compile the current tab</source>
            <translation>按 F7 编译当前标签页</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="175" />
            <source>&lt;b&gt;descriptors&lt;/b&gt; %1 &amp;nbsp; &lt;b&gt;push constants&lt;/b&gt; %2 &amp;nbsp; &lt;b&gt;inputs&lt;/b&gt; %3 &amp;nbsp; &lt;b&gt;outputs&lt;/b&gt; %4</source>
            <translation>&lt;b&gt;descriptors&lt;/b&gt; %1 &amp;nbsp; &lt;b&gt;推送常量&lt;/b&gt; %2 &amp;nbsp; &lt;b&gt;输入&lt;/b&gt; %3 &amp;nbsp; &lt;b&gt;输出&lt;/b&gt; %4</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="187" />
            <source>Execution</source>
            <translation>执行</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="194" />
            <source>Entry point</source>
            <translation>入口点</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="195" />
            <source>Execution model</source>
            <translation>执行模型</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="197" />
            <source>local size</source>
            <translation>local size</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="204" />
            <source>invocations</source>
            <translation>invocations</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="207" />
            <source>output vertices</source>
            <translation>output vertices</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="210" />
            <source>execution modes</source>
            <translation>execution modes</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="213" />
            <source>Generator</source>
            <translation>生成器</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="219" />
            <source>Descriptor Bindings</source>
            <translation>描述符绑定</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="232" />
            <source>(unnamed)</source>
            <translation>（未命名）</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="233" />
            <source>%1 bytes%2%3</source>
            <translation>%1 字节%2%3</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="237" />
            <source>  [unused]</source>
            <translation>  [未使用]</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="265" />
            <source>Push Constants</source>
            <translation>推送常量块</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="306" />
            <source>Stage Inputs</source>
            <translation>阶段输入</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="307" />
            <source>Stage Outputs</source>
            <translation>阶段输出</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="312" />
            <source>Specialization Constants</source>
            <translation>特化常量</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="324" />
            <source>(this module exposes no reflection data)</source>
            <translation>（此模块没有反射数据）</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="334" />
            <source>Reflection data copied to the clipboard (aligned plain-text tables).</source>
            <translation>反射数据已复制到剪贴板（对齐的纯文本表格）。</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="402" />
            <source>The current module has no UBO/SSBO to generate a struct from.

Compile a shader that declares a layout(set=, binding=) uniform/buffer block first.</source>
            <translation>当前模块没有可用于生成 struct 的 UBO/SSBO。\n\n请先编译一个声明了 layout(set=, binding=) uniform/buffer block 的着色器。</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="427" />
            <source>Descriptors were found, but they have no expandable block members (usually samplers or images).

Struct generation only applies to uniform buffers / storage buffers.</source>
            <translation>已找到 descriptors，但它们没有可展开的 block 成员（通常是采样器或图像）。\n\nstruct 生成仅适用于 uniform buffer / storage buffer。</translation>
        </message>
        <message>
            <location filename="../ui/panels/ReflectPanel.cpp" line="436" />
            <source>Generated structs: %1 (copied to the clipboard).

Types use glm::vec*/mat*; make sure glm is available in your project.</source>
            <translation>已生成 struct：%1（已复制到剪贴板）。\n\n类型使用 glm::vec*/mat*；请确保项目中可用 glm。</translation>
        </message>
    </context>
    <context>
        <name>spall::SettingsDialog</name>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="149" />
            <source>Settings</source>
            <translation>设置</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="154" />
            <source>Compilation</source>
            <translation>编译</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="155" />
            <source>Editor</source>
            <translation>编辑器</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="156" />
            <source>Appearance</source>
            <translation>外观</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="157" />
            <source>Interface</source>
            <translation>界面</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="174" />
            <source>Default #version</source>
            <translation>默认 #version</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="182" />
            <source>Vulkan client version</source>
            <translation>Vulkan 客户端版本</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="191" />
            <source>Target environment (validation/disassembly)</source>
            <translation>目标环境（校验/反汇编）</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="200" />
            <source>SPIR-V target version</source>
            <translation>SPIR-V 目标版本</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="202" />
            <source>Assign locations automatically</source>
            <translation>自动分配 location</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="205" />
            <source>Also numbers IO variables that lack layout(location=), so the module still validates.</source>
            <translation>同时为缺少 layout(location=) 的 IO 变量编号，使模块仍能通过校验。</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="209" />
            <source>Assign bindings/sets automatically</source>
            <translation>自动分配 binding/set</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="212" />
            <source>When this is off and the source has no layout(binding=), the module will fail SPIR-V validation — useful for exercising the validation path.</source>
            <translation>关闭此项且源码中没有 layout(binding=) 时，模块将无法通过 SPIR-V 校验——可用于测试校验流程。</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="216" />
            <source>Expand #include directives</source>
            <translation>展开 #include 指令</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="220" />
            <source>Validate SPIR-V by default</source>
            <translation>默认校验 SPIR-V</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="224" />
            <source>Inject tool macro (#define SPALL 1)</source>
            <translation>注入工具宏（#define SPALL 1）</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="229" />
            <source>One directory per line</source>
            <translation>每行一个目录</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="232" />
            <source>#include search paths</source>
            <translation>#include 搜索路径</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="243" />
            <source>Leave empty to use the system monospace font</source>
            <translation>留空则使用系统等宽字体</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="244" />
            <source>Font</source>
            <translation>字体</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="249" />
            <source>Font size</source>
            <translation>字号</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="254" />
            <source>Tab width</source>
            <translation>Tab 宽度</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="256" />
            <source>Insert spaces instead of tabs</source>
            <translation>用空格代替 Tab</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="260" />
            <source>Show completion popup while typing</source>
            <translation>输入时显示补全弹窗</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="264" />
            <source>Enable basic syntax hints (heuristic)</source>
            <translation>启用基础语法提示（启发式）</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="267" />
            <source>Checks run before compilation: bracket matching, missing semicolons, common typos. They are reported as hints and warnings only, not as full semantic analysis.</source>
            <translation>这些检查在编译前运行：括号匹配、缺失的分号、常见拼写错误。它们只作为提示和警告报告，不是完整的语义分析。</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="280" />
            <source>Follow system</source>
            <translation>跟随系统</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="281" />
            <source>Light</source>
            <translation>浅色</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="282" />
            <source>Dark</source>
            <translation>深色</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="284" />
            <source>Theme</source>
            <translation>主题</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="286" />
            <source>Hide unused variables in the reflection panel</source>
            <translation>在反射面板中隐藏未使用的变量</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="291" />
            <source>Window layout, geometry and the recent-files list are saved automatically and restored on the next start.</source>
            <translation>窗口布局、几何尺寸和最近文件列表会自动保存，并在下次启动时恢复。</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="312" />
            <source>Language</source>
            <translation>语言</translation>
        </message>
        <message>
            <location filename="../ui/SettingsDialog.cpp" line="315" />
            <source>The interface switches immediately; no restart is required. English is the source language and is always available.</source>
            <translation>界面会立即切换，无需重启。英文是源语言，始终可用。</translation>
        </message>
    </context>
</TS>