#ifndef _YAMNET_WRAPPER_H_
#define _YAMNET_WRAPPER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

	// 前向声明
	typedef struct yamnet_context yamnet_context_t;

	// 音频类别检测结果
	typedef struct
	{
		int cls_id;		  // 类别ID
		char cls_name[4]; // 大分类名称
		float confidence; // 置信度
	} yamnet_detect_result_t;

	// 检测结果列表
	typedef struct
	{
		int count;
		yamnet_detect_result_t results[10]; // 最多返回前10个结果
	} yamnet_detect_result_list_t;

	// YAMNet包装函数
	yamnet_context_t *yamnet_create_context(float threshold);
	int yamnet_init_model(yamnet_context_t *ctx, const char *model_path);
	int yamnet_inference(yamnet_context_t *ctx, const float *audio_data, int audio_length,
						 yamnet_detect_result_list_t *results);
	void yamnet_destroy_context(yamnet_context_t *ctx);

	// 获取类别名称
	const char *yamnet_get_class_name(int cls_id);

	// 初始化后处理
	int yamnet_init_post_process();

	// 清理后处理
	void yamnet_deinit_post_process(void);

	// 音频预处理函数
	int yamnet_preprocess_audio(const int16_t *pcm_data, int pcm_length, float *audio_data);

	// 检查是否检测到猫狗叫声
	int yamnet_check_cat_dog(yamnet_context_t *ctx, yamnet_detect_result_list_t *results);

#ifdef __cplusplus
}
#endif

#endif // _YAMNET_WRAPPER_H_