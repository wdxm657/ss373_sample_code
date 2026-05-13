# ive sample demo使用说明

仅支持在pure linux系统中运行

---

## 一、demo场景


此路径下的demo均为ive算子sample demo

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/ive/sample/*`命令进行全部的`ive sample demo`编译，或者`make source/ifado/ive/sample/xxx`指定算子demo;

3. 获取可执行文件：`sample_code/out/arm/app/prog_ive_sample_* `;

---

## 三、运行环境说明

* 无特殊环境要求

* 命名规则
  - `input_file = ./resource/input/ive/分辨率_文件格式_简约描述.bin`

  图像类output
  - `output_file = ./out/ive/Output_算子名称_分辨率_格式（_简约描述）.bin`

    如：`./out/ive/Output_Filter_1280x720_U8C1.bin`

  信息类output
  - `output_file = ./out/ive/Output_算子名称_结构体名称/大小（_简约描述）.bin`

    如：`./out/ive/Output_Hist_U32x256.bin` 和 `./out/ive/Output_EqualizeHist_EqualizeHistCtrlMem_t.bin`

---

## 四、运行说明

1. **filter**

   - 执行 5x5 模板滤波任务，通过配置不同的模板系数，可以实现不同的滤波。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_filter`

   - input/output:

     - `input_file1 = ./resource/input/ive/1280x720_U8C1_Img0.bin`

     - `output_file = ./out/ive/Output_Filter_1280x720_U8C1.bin`



2. **csc**

   - 执行YUV2RGB\YUV2BGR\RGB2YUV\BGR2YUV 等模式的色彩空间转换。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_csc`

   - input/output:

     - `input_file = ./resource/input/ive/1280x720_YUV420SP_Img1.bin`

     - `output_file = ./out/ive/Output_CSC_1280x720_RGB.bin`



3. **filter_csc**
   - 执行 5x5 模板滤波和 YUV2RGB 色彩空间转换复合任务，通过一次执行完成两种功能。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_filter_csc`

   - input/output:
     - `input_file = ./resource/input/ive/1280x720_YUV420SP_Img1.bin`

     - `output_file = ./out/ive/Output_FilterAndCSC_1280x720_RGB.bin`



4. **sobel**
   - 执行 5x5 模板 Sobel-like 梯度计算任务。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_sobel`

   - input/output:
     - `input_file = ./resource/input/ive/1280x720_U8C1_square.bin`

     - `output_file1 = ./out/ive/Output_Sobel_Horizontal_1280x720_S16C1.bin`
     - 垂直梯度 – 检测垂直方向上的梯度变化
     - `output_file2 = ./out/ive/Output_Sobel_Vertical_1280x720_S16C1.bin`
     - 水平梯度 – 检测水平方向上的梯度变化



5. **mag_and_ang**
   - 执行 5x5 模板梯度幅值与幅角计算任务。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_mag_and_ang`

   - input/output:

     - `input_file1 = ./resource/input/ive/1280x720_U8C1_square.bin`

     - `output_file1 = ./out/ive/Output_MagAndAng_Mag_1280x720_U16C1.bin`
     - `output_file2 = ./out/ive/Output_MagAndAng_Ang_1280x720_U8C1.bin`

      output_file2为幅角，为较小的数，以图像形式无法直观看出。



6. **dilate**

   - 执行二值图像 5x5 模板膨胀任务。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_dilate`

   - input/output:

     - `input_file = ./resource/input/ive/1280x720_U8C1_rhombus_binary.bin`

     - `output_file = ./out/ive/Output_Dilate_1280x720_U8C1.raw`



7. **erode**

   - 执行二值图像 5x5 模板腐蚀任务。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_erode`

   - input/output:

     - `input_audio = ./resource/input/ive/1280x720_U8C1_rhombus_binary.bin`

     - `output_file = ./out/ive/Output_Erode_1280x720_U8C1.bin`



8. **thresh**

   - 执行灰度图像阈值化任务。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_thresh`

   - input/output:

     - `input_file = ./resource/input/ive/1280x720_U8C1_gray_scale.bin`

     - `output_file = ./out/ive/Output_Thresh_1280x720_U8C1.bin`



9. **and**

   - 执行两二值图像相与任务。详细描述可参考MI_IVE API文档。

   - Usage:  `./prog_ive_sample_and`

   - input/output:

     - `input_file1 = ./resource/input/ive/1280x720_U8C1_round.bin`
     - `input_file2 = ./resource/input/ive/1280x720_U8C1_square.bin`

     - `output_file = ./out/ive/Output_And_1280x720_U8C1.bin`




10. **sub**

    - 执行两灰度图像相减任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_sub`

    - input/output:
      - `input_file1 = ./resource/input/ive/1280x720_U8C1_Img0.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_round_64.bin`

      - `output_file = ./out/ive/Output_Sub_1280x720_U8C1.bin`




11. **or**

    - 执行两二值图像相或任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_or`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_U8C1_round.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_square.bin`

      - `output_file = ./out/ive/Output_Or_1280x720_U8C1.bin`




12. **integ**

    - 执行灰度图像的积分图计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_integ`

    - input/output:
      - input_file = ./resource/input/ive/1280x720_U8C1_all_2.bin

      - `input为像素值为全2的1280x720的灰度值图像`

      - `output_file = ./out/ive/Output_Integ_1280x720_U64C1.bin`

      - ctrl mode = E_MI_IVE_INTEG_OUT_CTRL_SUM，output为像素值由2不断累加的1280x720的U64C1图像




13. **hist**
    - 执行灰度图像的直方图统计任务。详细描述可参考MI_IVE API文档。

    - Usage: `./prog_ive_sample_hist`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_Img0.bin`

      - `output_file = ./out/ive/Output_Hist_U32x256.bin`

      Output_Hist_U32x256.bin 为各亮度的统计值，非可观察图像




14. **thresh_s16**
    - 执行 S16 数据到 8bit 数据的阈值化任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_thresh_s16`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_gray_scale_16bit.bin`

      - `output_file = ./out/ive/Output_Thresh_S16_1280x720_U8C1.bin`




15. **thresh_u16**
    - 执行 U16 数据到 U8 数据的阈值化任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_thresh_u16`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_gray_scale_16bit.bin`

      - `output_file = ./out/ive/Output_Thresh_U16_1280x720_U8C1.bin`




16. **16bit_to_8bit**

    - 执行 16bit 图像数据到 8bit 图像数据的线性转化任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_16bit_to_8bit`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_gray_scale_16bit.bin`

      - `output_file = ./out/ive/Output_16BitTo8Bit_1280x720_U8C1.bin`




17. **order_statistic_filter**

    - 执行 3x3 模板顺序统计量滤波任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_order_statistic_filter`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_Salt.bin`

      - `output_file = ./out/ive/Output_OrdStatFilter_1280x720_U8C1.bin`




18. **map**

    - 执行 Map（映射赋值）任务，对源图像中的每个像素，查找 Map查找表中的值，赋予目标图像相应像素查找表中的值，支持 U8C1U8C1模式的映像。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_map`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_Img0.bin`

      - `output_file = ./out/ive/Output_Map_1280x720_U8C1.bin`

    映射关系为 0->255, 1->254......254->1, 0->255 的map，output为应用该map重新映射后的图像，效果呈现为反色。



19. **equalize_hist**

    - 执行灰度图像的直方图均衡化计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_equalize_hist`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_Img0.bin`

      - `output_file1 = ./out/ive/Output_EqualizeHist_1280x720_U8C1.bin`
      - `output_file2 = ./out/ive/Output_EqualizeHist_EqualizeHistCtrlMem_t.bin`

      output2 为各灰度值的统计结果，无法直接观测，需要按照 MI_IVE_EqualizeHistCtrlMem_t 格式解析。




20. **add**

    - 执行两灰度图像的加权加计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_add`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_U8C1_Img0.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_round_64.bin`

      - `output_file = ./out/ive/Output_Add_1280x720_U8C1.bin`



21. **xor**

    - 执行两二值图的异或计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_order_statistic_filter`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_U8C1_round.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_square.bin`

      - `output_file = ./out/ive/Output_Xor_1280x720_U8C1.bin`



22. **ncc**

    - 执行两相同分辨率灰度图像的归一化互相关系数计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_ncc`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_U8C1_outdoor_f00.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_outdoor_f01.bin`

      - `output_file = ./out/ive/Output_NCC_NccDstMem_t.bin`

    根据 MI_IVE_NccDstMem_t 结构体解析output信息，根据MI_IVE API文档计算两张input图像相关性



23. **ccl**

    - 执行二值图像的连通区域标记任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_ccl`

    - input/output:

      - `input_file = ./resource/input/ive/16x4_U8C1_CCL.bin`

      - `output_file1 = ./out/ive/Output_CCL_16x4_U8C1_grid.bin`
      - `output_file2 = ./out/ive/Output_CCL_CcBlob_t.bin`

      将CCL input 和 ouput1 对比查看可观察连通计算情况，output2需要按照 MI_IVE_CcBlob_t 结构体解析。



24. **gmm**

    - 执行 GMM 背景建模任务，支持灰度图、RGB_PACKAGE 图像的 GMM 背景建模，高斯模型个数为 3 或者 5。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_gmm`

    - input/output:

      - `input_file = ./resource/input/ive/352x288_U8C1_Imgx50.bin`

      - `output_file = ./out/ive/Output_GMM_352x288_U8C1.bin`

    GMM output 为多张连续的图像，需连续比较效果。


25. **canny**

    - 灰度图的 Canny边缘提取的前半部：求梯度、计算梯度幅值幅角、磁滞阈值化及非极大抑制。

    - 灰度图的 Canny 边缘提取的后半部：连接边缘点，形成 Canny 边缘图。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_canny`

    - input/output:

      - `input_file = ./resource/input/ive/640x480_U8C1_Img1.bin`

      - `output_file = ./out/ive/Output_Canny_640x480_U8C1.bin`



26. **lbp**

    - 执行 LBP 局部二值模式计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_lbp`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_Img0.bin`

      - `output_file = ./out/ive/Output_LBP_1280x720_U8C1.bin`



27. **normal_gradient**

    - 执行归一化梯度计算任务，梯度分量均归一化到 S8。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_normal_gradient`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_square.bin`

      - `output_file1 = ./out/ive/Output_NormGrad_Horizontal_1280x720_S8C1.bin`
      - `output_file2 = ./out/ive/Output_NormGrad_Vertical_1280x720_S8C1.bin`
      - `output_file3 = ./out/ive/Output_NormGrad_Combine_1280x720_S8C2.bin`



28. **lk_optical_flow**

    - 执行单层 LK 光流计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_lk_optical_flow`

    - input/output:

      - `input_file1 = ./resource/input/ive/160x120_U8C1_Lksrc1.bin`
      - `input_file2 = ./resource/input/ive/160x120_U8C1_Lksrc2.bin`

      - `output_file = ./out/ive/Output_Lk_Optical_Flow_MvS9Q7_t.bin`

      Output_Lk_Optical_Flow_MvS9Q7_t.bin 输出为运动矢量数据，非可观察图像，需要根据 MI_IVE_MvS9Q7_t 结构体解析




29. **sad**

    - 计算两幅图像按 4x4/8x8/16x16 分块的 16 bit/8 bit SAD 图像，以及对 SAD 进行阈值化输出。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_sad`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_U8C1_outdoor_f00.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_outdoor_f10.bin`

      - `output_file1 = ./out/ive/Output_SAD_Sad_160x90_U16C1.bin`
      - `output_file2 = ./out/ive/Output_SAD_Thr_160x90_U8C1.bin`

      8x8分块模式下，output1为 16bit SAD 图像，output2为对 SAD 图像进行阈值化的输出


30. **bernsen**

    - 执行 3x3 和 5x5 模板的 Bernsen 门坎值任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_bernsen`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_text.bin`

      - `output_file = ./out/ive/Output_Bernsen_1280x720_U8C1.bin`

      使用3x3模板，输出为0和1分布的1280x720的二值化图像，可通过 阈值处理 thresh 处理得到可预览图



31. **line_filter_hor**

    - 针对二位图像进行水平方向的滤波任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_line_filter_hor`

    - input/output:

      - `input_file = ./resource/input/ive/640x480_U8C1_Img1.bin`

      - `output_file = ./out/ive/Output_LineFilterHor_640x480_U8C1.bin`


32. **line_filter_ver**

    - 针对二位图像进行垂直方向的滤波任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_line_filter_ver`

    - input/output:

      - `input_file = ./resource/input/ive/640x480_U8C1_Img1.bin`

      - `output_file = ./out/ive/Output_LineFilterVer_640x480_U8C1.bin`


33. **noise_remove_hor**

    - 针对二位图像进行水平方向之噪声滤除任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_noise_remove_hor`

    - input/output:

      - `input_file = ./resource/input/ive/640x480_U8C1_Img1.bin`

      - `output_file = ./out/ive/Output_NoiseRemoveHor_640x480_U8C1.bin`


34. **noise_remove_ver**

    - 针对二位图像进行水平方向之噪声滤除任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_noise_remove_ver`

    - input/output:

      - `input_file = ./resource/input/ive/640x480_U8C1_Img1.bin`

      - `output_file = ./out/ive/Output_NoiseRemoveVer_640x480_U8C1.bin`


35. **adp_thresh**

    - 执行使用自适应性门坎值的二值化任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_adp_thresh`

    - input/output:

      - `input_file = ./resource/input/ive/640x480_U8C1_Img1.bin`

      - `output_file = ./out/ive/Output_Adpthresh_640x480_U8C1.bin`


36. **resize**

    - 执行缩放影像任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_resize`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_YUV420SP_Img1.bin`

      - `output_file = ./out/ive/Output_Resize_320x360_YUV420sp.bin`


37. **bat**

    - 针对二位图像执行水平和垂直方向的数值交替次数计算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_bat`

    - input/output:

      - `input_file = ./resource/input/ive/1280x720_U8C1_rhombus_binary.bin`

      - `output_file1 = ./out/ive/Output_Bat_Out_720_dst_h.bin`
      - `output_file2 = ./out/ive/Output_Bat_Out_1280_dst_v.bin`
        ```
        .u16HorTimes = 640,
        .u16VerTimes = 360,
        ```

      阈值设置推荐为输入图像分辨率各一半，水平和竖直方向侦测阈值为分辨率一半情况下：

      - 水平方向输出为每一行的数值交替次数与设定阈值做比较，超过阈值则该行输出为1，共组成720个0/1的值。

      - 竖直方向输出为每一列的数值交替次数与设定阈值做比较，超过阈值则该行输出为1，共组成1280个0/1的值。



38. **acc**

    - 执行两灰度图像的累积运算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_acc`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_U8C1_square.bin`
      - `input_file2 = ./resource/input/ive/1280x720_U8C1_round.bin `

      - `output_file = ./out/ive/Output_Acc_1280x720_U8C1.bin`


39. **matrix_transform**

    - 执行矩阵运算任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_matrix_transform`

    - input/output:

      - `input_file1 = ./resource/input/ive/640x320_S32C1_src.bin`
      - `input_file2 = ./resource/input/ive/640x320_S32C1_src.bin`
      - `input_file3 = ./resource/input/ive/640x320_S32C1_src.bin`

      - `output_file1 = ./out/ive/Output_matrix_transform_640x320_S32C1_dts1.bin`
      - `output_file2 = ./out/ive/Output_matrix_transform_640x320_S32C1_dts2.bin`
      - `output_file3 = ./out/ive/Output_matrix_transform_640x320_S32C1_dts3.bin`

      用于数值计算，本身不作为图像显示


40. **image_dot**

    - 执行图像点乘积任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_image_dot`

    - input/output:

      - `input_file1 = ./resource/input/ive/1280x720_S32C1_Dotsrc.bin`
      - `input_file2 = ./resource/input/ive/1280x720_S32C1_Dotsrc.bin `

      - `output_file = ./out/ive/Output_Image_Dot_1280x720_S32C1.bin`

      用于数值计算，本身不作为图像显示



41. **shift_detector**

    - 执行图像区域追踪任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_shift_detector`

    - input/output:

      - `input_file1 = ./resource/input/ive/352x288_U8C1_f09.bin`
      - `input_file2 = ./resource/input/ive/352x288_U8C1_f15.bin `

      - `output_file1 = ./out/ive/Output_shift_detector_MV_X_1x1_U16C1.bin`
      - `output_file2 = ./out/ive/Output_shift_detector_MV_X_1x1_U16C1.bin`

      ctrl.enMode = E_MI_IVE_SHIFT_DETECT_MODE_SINGLE的情况下，输出的bin为1x1大小的date，分别为单一追踪物体前后的x、y坐标变化情况



42. **alpha_blending**

    - 执行图像个别权重乘积任务。详细描述可参考MI_IVE API文档。

    - Usage:  `./prog_ive_sample_alpha_blending`

    - input/output:

      - `input_file1 = ./resource/input/ive/640x360_YUV420SP_sori.bin`
      - `input_file2 = ./resource/input/ive/640x360_YUV420SP_bg.bin `
      - `input_file3 = ./resource/input/ive/640x360_U8C1_ymask.bin`

      - `output_file = ./out/ive/Output_AlphaBlending_640x360_U8C1.bin`

      alpha(input3)图的像素值作为权重，基于alpha图，将src0和src1的内容融合在一起

