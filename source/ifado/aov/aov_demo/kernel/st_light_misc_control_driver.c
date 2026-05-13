/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeirq.h>
#include <linux/regmap.h>
#include <linux/stmmac.h>
#include <linux/notifier.h>
#include <linux/syscalls.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/suspend.h>

#include "light_misc_control_main.h"

extern int tig_mode;

static int light_misc_control_dev_open(struct inode *inode, struct file *file)
{
    /* printk(KERN_INFO "Driver Opened\n"); */
    return 0;
}

static int light_misc_control_dev_release(struct inode *inode, struct file *file)
{
    /* printk(KERN_INFO "Driver Closed\n"); */
    return 0;
}

static long light_misc_control_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd)
    {
        case IOCTL_LIGHT_MISC_CONTROL_INIT:
        {
            SSTAR_Light_Misc_Callback_Param_t stCallback_Param = {0};
            if (copy_from_user(&stCallback_Param, (int __user *)arg, sizeof(SSTAR_Light_Misc_Callback_Param_t)))
            {
                return -EFAULT;
            }
            if (!SSTAR_Light_Misc_Control_IOCTL_Init(stCallback_Param.vifDevId ,(void **)&stCallback_Param.fun_ptr_addr))
            {
                if (copy_to_user((int __user *)arg, &stCallback_Param, sizeof(SSTAR_Light_Misc_Callback_Param_t)))
                {
                    return -EFAULT;
                }
            }
            else
            {
                return -EFAULT;
            }
        }
        break;
        case IOCTL_LIGHT_MISC_CONTROL_DEINIT:
            if (SSTAR_Light_Misc_Control_IOCTL_DeInit(true))
            {
                return -EFAULT;
            }
            break;
        case IOCTL_LIGHT_MISC_CONTROL_SET_ATTR:
        {
            SSTAR_Light_Ctl_Attr_t stLight_Attr = {0};
            if (copy_from_user(&stLight_Attr, (int __user *)arg, sizeof(SSTAR_Light_Ctl_Attr_t)))
            {
                return -EFAULT;
            }
            if (SSTAR_Light_Misc_Control_IOCTL_Set_Attr(&stLight_Attr))
            {
                return -EFAULT;
            }
        }
        break;
        case IOCTL_LIGHT_MISC_CONTROL_GET_ATTR:
        {
            SSTAR_Light_Ctl_Attr_t stLight_Attr = {0};
            if (SSTAR_Light_Misc_Control_IOCTL_Get_Attr(&stLight_Attr))
            {
                return -EFAULT;
            }
            if (copy_to_user((int __user *)arg, &stLight_Attr, sizeof(SSTAR_Light_Ctl_Attr_t)))
            {
                return -EFAULT;
            }
        }
        break;
        case IOCTL_LIGHT_MISC_CONTROL_SET_IRCUT:
        {
            int ircutState;
            if (copy_from_user(&ircutState, (int __user *)arg, sizeof(int)))
            {
                return -EFAULT;
            }
            if (SSTAR_Light_Misc_Control_IOCTL_Set_Ircut_State(ircutState))
            {
                return -EFAULT;
            }
        }
        break;
        case IOCTL_LIGHT_MISC_CONTROL_GET_LIGHTSENSOR:
        {
            int lightSensorVal;
            lightSensorVal = SSTAR_Light_Misc_Control_IOCTL_Get_LightSensor();
            if (copy_to_user((int __user *)arg, &lightSensorVal, sizeof(int)))
            {
                return -EFAULT;
            }
        }
        break;
        case IOCTL_LIGHT_MISC_CONTROL_GET_TIGMODE:
            if (copy_to_user((int __user *)arg, &tig_mode, sizeof(int)))
            {
                return -EFAULT;
            }
            break;
        default:
            ret = -ENOTTY;
            break;
    }

    return ret;
}

#define BUFFER_SIZE 128

static ssize_t light_misc_control_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    size_t ret                       = 0;
    char   debugBuf[BUFFER_SIZE * 2] = {0};

    if (buf == NULL)
    {
        pr_err("buf does not exist, read fail\n");
    }

    if (SSTAR_Light_Misc_Control_Debug_Get_State(debugBuf))
    {
        return -EFAULT;
    }

    if (*offset != 0)
    {
        return 0;
    }

    ret = simple_read_from_buffer(buf, count, offset, debugBuf, strlen(debugBuf));

    if (ret > 0)
    {
        *offset = ret;
    }

    return ret;
}

static ssize_t light_misc_control_dev_write(struct file *filp, const char __user *user_buffer, size_t count,
                                            loff_t *offset)
{
    char  buffer[BUFFER_SIZE] = {0};
    int   len                 = 0;
    char *token;
    char *buf;

    if (count >= BUFFER_SIZE)
    {
        pr_err("Input too long\n");
        return -EINVAL;
    }
    if (user_buffer == NULL)
    {
        pr_err("user_buffer does not exist, write fail\n");
        return -EFAULT;
    }
    if (copy_from_user(buffer, user_buffer, count))
    {
        pr_err("Failed to copy data from user\n");
        return -EFAULT;
    }

    buf = buffer;

    len      = strcspn(buf, "\n");
    buf[len] = '\0';

    pr_info("echo command:%s\n", buf);
    if((token = strsep((char **)&buf, " ")) == NULL)
    {
        goto HELP;
    }
    if (!token)
    {
        pr_err("No command found\n");
        kfree(buf);
        return -EINVAL;
    }

    if (strcmp(token, "getlux") == 0)
    {
        pr_info("lux:%d\n", SSTAR_Light_Misc_Control_IOCTL_Get_LightSensor());
    }
    else if (strcmp(token, "setircut") == 0)
    {
        if((token = strsep((char **)&buf, " ")) == NULL)
        {
            goto HELP;
        }
        SSTAR_Light_Misc_Control_IOCTL_Set_Ircut_State(strcmp(token, "on") == 0 ? E_SWITCH_STATE_ON
                                                                                : E_SWITCH_STATE_OFF);
        msleep(100);
        SSTAR_Light_Misc_Control_IOCTL_Set_Ircut_State(E_SWITCH_STATE_KEEP);
    }
    else if (strcmp(token, "setlight") == 0)
    {
        if((token = strsep((char **)&buf, " ")) == NULL)
        {
            goto HELP;
        }
        if (strcmp(token, "ir") == 0)
        {
            SSTAR_Light_Ctl_Attr_t stLight_Attr = {0};

            if((token = strsep((char **)&buf, " ")) == NULL)
            {
                goto HELP;
            }
            stLight_Attr.lightType = E_LIGHT_TYPE_IR;
            if (strcmp(token, "multi") == 0)
            {
                int ret, value;
                stLight_Attr.controlType = E_CONTROL_TYPE_MULTI_FRAME;
                if((token = strsep((char **)&buf, " ")) == NULL)
                {
                    goto HELP;
                }
                ret                      = kstrtoint(token, 10, &value);
                if (ret)
                {
                    pr_err("timeMs:%s param is err\n", token);
                    goto HELP;
                }
                stLight_Attr.delayOpenTimeMs = value;
            }
            else
            {
                stLight_Attr.controlType =
                    strcmp(token, "on") == 0 ? E_CONTROL_TYPE_LONG_TERM_ON : E_CONTROL_TYPE_LONG_TERM_OFF;
            }
            SSTAR_Light_Misc_Control_IOCTL_Set_Attr(&stLight_Attr);
        }
        else if (strcmp(token, "led") == 0)
        {
            SSTAR_Light_Ctl_Attr_t stLight_Attr = {0};

            if((token = strsep((char **)&buf, " ")) == NULL)
            {
                goto HELP;
            }
            stLight_Attr.lightType = E_LIGHT_TYPE_LED;
            if (strcmp(token, "multi") == 0)
            {
                int ret, value;
                stLight_Attr.controlType = E_CONTROL_TYPE_MULTI_FRAME;
                if((token = strsep((char **)&buf, " ")) == NULL)
                {
                    goto HELP;
                }
                ret                      = kstrtoint(token, 10, &value);
                if (ret)
                {
                    pr_err("timeMs:%s param is err\n", token);
                    goto HELP;
                }
                stLight_Attr.delayOpenTimeMs = value;
            }
            else
            {
                stLight_Attr.controlType =
                    strcmp(token, "on") == 0 ? E_CONTROL_TYPE_LONG_TERM_ON : E_CONTROL_TYPE_LONG_TERM_OFF;
            }
            SSTAR_Light_Misc_Control_IOCTL_Set_Attr(&stLight_Attr);
        }
        else
        {
            pr_err("command args:%s fail\n", token);
            pr_info("Remaining buffer: %s\n", buf);
        }
    }
    else if (strcmp(token, "init") == 0)
    {
        void *ptrCallBack;
        SSTAR_Light_Misc_Control_IOCTL_Init(-1, &ptrCallBack);
    }
    else if (strcmp(token, "debuglog") == 0)
    {
        if((token = strsep((char **)&buf, " ")) == NULL)
        {
            goto HELP;
        }
        SSTAR_Light_Misc_Control_Set_DebugLog(strcmp(token, "on") == 0);
    }
    else
    {
    HELP:
        pr_info("help info:\n");
        pr_info("-----------------------------------------------------\n");
        pr_info("echo init > /dev/light_misc\n");
        pr_info("echo getlux > /dev/light_misc\n");
        pr_info("echo setircut on/off > /dev/light_misc\n");
        pr_info("echo setlight ir/led on/off > /dev/light_misc\n");
        pr_info("echo setlight ir/led multi timeMs > /dev/light_misc\n");
        pr_info("echo debuglog on/off > /dev/light_misc\n");
        pr_info("-----------------------------------------------------\n");
    }

    return count;
}

static const struct file_operations light_misc_control_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = light_misc_control_dev_open,
    .release        = light_misc_control_dev_release,
    .write          = light_misc_control_dev_write,
    .read           = light_misc_control_dev_read,
    .unlocked_ioctl = light_misc_control_dev_ioctl,
};

static struct miscdevice light_misc_control_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "light_misc",
    .fops  = &light_misc_control_dev_fops,
};

static int sstar_light_misc_control_probe(struct platform_device *pdev)
{
    if (!(pdev->name) || strcmp(pdev->name, "soc:light_misc_control")) // || pdev->id != 0)
    {
        pr_err("prob [%s] name:%s error! pdev->id:%d\n", __FUNCTION__, pdev->name, pdev->id);
        return -ENXIO;
    }
    pr_info("prob [%s] name:%s\n", __FUNCTION__, pdev->name);
    return SSTAR_Light_Misc_Control_Resource_Config(pdev);
}

static int sstar_light_misc_control_remove(struct platform_device *pdev)
{
    pr_info("remove [%s]\n", __FUNCTION__);
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_light_misc_control_pm_suspend(struct device *dev)
{
    SSTAR_Light_Misc_Control_Suspend();
    /* pr_info("[%s]\n", __FUNCTION__); */
    return 0;
}
static int sstar_light_misc_control_pm_resume_noirq(struct device *dev)
{
    SSTAR_Light_Misc_Control_Resume_Notify_Main(dev);
    pr_info("[%s]\n", __FUNCTION__);
    return 0;
}

static int sstar_light_misc_control_pm_resume(struct device *dev)
{
    SSTAR_Light_Misc_Control_Resume();
    /* pr_info("[%s]\n", __FUNCTION__); */
    return 0;
}

static int sstar_light_misc_control_pm_resume_early(struct device *dev)
{
    /* pr_info("[%s]\n", __FUNCTION__); */
    return 0;
}

struct dev_pm_ops sstar_light_misc_control_pm_ops = {
    .suspend      = sstar_light_misc_control_pm_suspend,
    .resume_noirq = sstar_light_misc_control_pm_resume_noirq,
    .resume       = sstar_light_misc_control_pm_resume,
    .resume_early = sstar_light_misc_control_pm_resume_early,
};

#endif /* CONFIG_PM_SLEEP */

static const struct of_device_id sstar_light_misc_control_match[] = {{.compatible = "sstar,light_misc_control"}, {}};
MODULE_DEVICE_TABLE(of, sstar_light_misc_control_match);

struct platform_driver sstar_light_misc_control_driver = {
    .probe  = sstar_light_misc_control_probe,
    .remove = sstar_light_misc_control_remove,
    .driver = {.name = "light_misc_control",
#ifdef CONFIG_PM_SLEEP
               .pm = &sstar_light_misc_control_pm_ops,
#endif
               .of_match_table = sstar_light_misc_control_match,
               .owner          = THIS_MODULE},
};

#ifdef LIGHT_USE_PM_NOTIFY
static int sstar_light_misc_control_pm_notify(struct notifier_block *b, unsigned long v, void *d)
{
    switch (v)
    {
        case PM_SUSPEND_PREPARE:
            pr_info("[light_misc_control] PM_SUSPEND_PREPARE\n");
            break;
        case PM_POST_SUSPEND:
            pr_info("[light_misc_control] PM_POST_SUSPEND:\n");
            break;
        default:
            return NOTIFY_DONE;
    }

    return NOTIFY_OK;
}

static struct notifier_block sstar_pm_notify_block = {
    .notifier_call = sstar_light_misc_control_pm_notify,
};
#endif

static int __init sstar_light_misc_control_module_init(void)
{
    int retval = 0;

    retval = platform_driver_register(&sstar_light_misc_control_driver);
    if (retval)
    {
        pr_err("light control module register failed %d\n", retval);
        return retval;
    }
    retval = misc_register(&light_misc_control_miscdev);
    if (retval)
    {
        pr_err("Failed to register LED device\n");
        return retval;
    }

    SSTAR_Light_Misc_Control_Init();

#ifdef LIGHT_USE_PM_NOTIFY
    register_pm_notifier(&sstar_pm_notify_block);
#endif
    return 0;
}

static void __exit sstar_light_misc_control_module_exit(void)
{
    pr_info("light control module exit\n");
    platform_driver_unregister(&sstar_light_misc_control_driver);

    misc_deregister(&light_misc_control_miscdev);

    SSTAR_Light_Misc_Control_DeInit();
#ifdef LIGHT_USE_PM_NOTIFY
    unregister_pm_notifier(&sstar_pm_notify_block);
#endif
    return;
}

module_init(sstar_light_misc_control_module_init);
module_exit(sstar_light_misc_control_module_exit);

MODULE_AUTHOR("Sigmastar");
MODULE_DESCRIPTION("aov light misc control driver");
MODULE_LICENSE("GPL v2");
