# PA1报告

这是由*沈溯 18307130102*和*赵文轩 18307130104*共同完成的复旦大学2021秋季学期《编译》课程的第一项编程作业。本次作业的目的是使用flex实现一个词法分析的工具。本次作业的具体任务是修改`cool.flex`中的代码，以实现一个能够对cool语言进行词法分析的词法分析器。

## 定义

词法分析器存在以下几个状态，分别为 INITIAL（初始状态），COMMENT（注释检测状态），STRING（字符串检测状态），STRING_ESCAPE（字符串转义检测状态）。

自己额外定义的变量有，comment_dep（检测注释嵌套的层数），char_tmp（用于字符串转义检测状态下暂存转义出来的字符），skip_char（当字符串过长时，不对当前读到的字符进行缓存）。

## 变量及关键词检测

OBJECT：开头小写字母或下划线，后面为大小写字母，数字或下划线。

TYPE：开头大写字母或下划线，后面为大小写字母，数字或下划线。

关键词不区分大小写，因此使用 ?i: 作为前缀。

EOL 的定义为了兼容 Windows 下的文件格式，定义为 (\r)?\n 。

true 和 false 要求首字母小写，其他部分不做要求。

非法字符包括 [ ] ' > &。

## 注释检测

INITIAL 状态下匹配到 (* 字符表示注释的开始，进入 COMMENT 状态。此时 comment_dep 必定为 1，否则上一次注释状态并不会退出。

COMMENT 状态下再次匹配到 (* 字符表示注释嵌套层数加 1，匹配到 *) 表示注释嵌套层数减 1。当注释嵌套层数，即 comment_dep 等于 0 时，退出 COMMENT 状态，回到 INITIAL 状态。

除此之外的所有字符都视作注释，如果匹配到 \n 字符还需要将 curr_lineno 加 1。

## 字符串检测

INITIAL 状态下匹配到 “ 字符，进入 STRING 状态，初始化字符串缓存指针位置为初始点，skip_char 设置为 0。

STRING 状态下匹配到 ” 字符，如果 skip_char 为 0，记录字符串信息，返回 STR_CONST。最后返回 INITIAL 状态。

STRING 状态下匹配到 \ 字符，进入 STRING_ESCAPE 状态。

STRING_ESCAPE 状态下，对于之后的字符进行转义操作，首先设置 char_tmp 为之后的字符，再针对 n b t \ f 几个字符进行特别判定，\n 另外判定，需要将 curr_lineno 加 1。STRING_ESCAPE 状态下匹配到一个字符之后，返回 STRING 状态。

## 异常检测

异常包括以下集中情况：

1. INITIAL 状态下匹配到 *)。
2. COMMENT 状态下匹配到 EOF。
3. STRING 状态下匹配到 EOF。
4. STRING 状态下匹配到 EOL(换行符)，EOL 需要在 STRING_ESCAPE 状态下匹配才能生效。
5. STRING 状态下匹配到 \0 字符。
6. 向字符串缓存中加入字符时溢出。

## 测试文件说明

test-01.cl ~ test-06.cl 是自己编写的测试文件，tes1.cl ~ test4.cl 是其他同学提供的。下面仅说明 test-01.cl ~ test-06.cl。

test-01.cl: 检测非法字符 [ ]。

test-02.cl: 检测异常4，字符串中未加转义符的换行。

test-03.cl: 检测异常6，字符串过长。

test-04.cl: 检测异常3，字符串中出现 EOF。

test-05.cl: 文档中关于字符串 \0 转义问题的特例，以及 true 和 false 的检测。

test-06.cl: 未封闭的注释，以及异常1，非注释中出现 *）。
