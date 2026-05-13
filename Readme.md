sample_code下分为子项目、lib，两部分，子项目以文件夹区分，如下所示：

子项目包括：source/internal/，统一由一套makefile管理

    verify

        |-----prebuild_libs //第三方开源库，或者是sigmastar开发的闭源库，库的存放位置按照toolchain划分，例如glibc-9.1.0的版本： lib/glibc/9.1.0

        |-----common        // 项目开发中使用到公共代码，与mi无关，与chip无关，例如rtsp

        |-----sample_code

            |-------->source     // 主要是各个模块demo代码参考

            |-------->internal   //各个子项目与mi相关的公共代码

            |--------Makefile    //全局的makefile，可以管理所有的子项目

            |--------build.mk    //makefile文件，管理include文件，lib库相关

            |--------compile.mk  //makefile文件，管理编译相关

            |--------mi_dep.mk   //makefile文件，管理参数定义相关

            |--------out

                |--------app    //编译出来的临时文件夹，存放编译输出的结果。

                |--------lib    //存放编译common和internal生成lib