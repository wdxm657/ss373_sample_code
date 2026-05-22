#ifndef DOG_SOOTHER_BARK_DETECT_H_
#define DOG_SOOTHER_BARK_DETECT_H_

int bark_detect_init(void);
void bark_detect_deinit(void);

/* 安抚措施执行期间暂停 YAMNet 推理（仍消费队列以防积压） */
void bark_detect_set_active(int active);

#endif /* DOG_SOOTHER_BARK_DETECT_H_ */
