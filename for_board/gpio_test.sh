#!/bin/sh

# 简单的Linux GPIO控制脚本 - 使用sysfs接口
# 用法: ./gpio_ctl.sh <gpio编号> <方向> [值]

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# GPIO基础路径
GPIO_BASE="/sys/class/gpio"

# 显示帮助信息
usage() {
    echo -e "${BLUE}GPIO控制脚本${NC}"
    echo ""
    echo "用法:"
    echo "  $0 <gpio> out <0|1>     # 设置为输出并设置电平"
    echo "  $0 <gpio> in            # 设置为输入"
    echo "  $0 <gpio> read          # 读取当前值"
    echo "  $0 <gpio> status        # 查看GPIO状态"
    echo "  $0 <gpio> unexport      # 取消导出GPIO"
    echo ""
    echo "示例:"
    echo "  $0 18 out 1             # GPIO18输出高电平"
    echo "  $0 18 out 0             # GPIO18输出低电平"
    echo "  $0 18 in                # GPIO18设置为输入"
    echo "  $0 18 read              # 读取GPIO18的值"
    echo "  $0 18 status            # 查看GPIO18状态"
    echo ""
}

# 检查是否root
check_root() {
    if [ "$EUID" -ne 0 ]; then 
        echo -e "${RED}错误: 请使用root权限运行${NC}"
        echo "使用: sudo $0 ..."
        exit 1
    fi
}

# 导出GPIO
export_gpio() {
    local gpio=$1
    local gpio_path="${GPIO_BASE}/gpio${gpio}"
    
    # 检查GPIO是否已经导出
    if [ ! -d "$gpio_path" ]; then
        echo -n "$gpio" > "${GPIO_BASE}/export" 2>/dev/null
        if [ $? -ne 0 ]; then
            echo -e "${RED}错误: 无法导出GPIO ${gpio}${NC}"
            echo "可能原因: GPIO不存在或已被占用"
            exit 1
        fi
        # 等待sysfs创建
        sleep 0.1
        echo -e "${GREEN}✓ GPIO ${gpio} 已导出${NC}"
    else
        echo -e "${YELLOW}GPIO ${gpio} 已经导出${NC}"
    fi
}

# 取消导出GPIO
unexport_gpio() {
    local gpio=$1
    echo -n "$gpio" > "${GPIO_BASE}/unexport" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ GPIO ${gpio} 已取消导出${NC}"
    else
        echo -e "${RED}错误: 无法取消导出GPIO ${gpio}${NC}"
    fi
}

# 设置GPIO方向
set_direction() {
    local gpio=$1
    local direction=$2
    local gpio_path="${GPIO_BASE}/gpio${gpio}"
    
    echo -n "$direction" > "${gpio_path}/direction" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ 方向设置为: ${direction}${NC}"
    else
        echo -e "${RED}错误: 无法设置方向${NC}"
        exit 1
    fi
}

# 设置GPIO值
set_value() {
    local gpio=$1
    local value=$2
    local gpio_path="${GPIO_BASE}/gpio${gpio}"
    
    # 验证值
    if [ "$value" != "0" ] && [ "$value" != "1" ]; then
        echo -e "${RED}错误: 值必须是0或1${NC}"
        exit 1
    fi
    
    echo -n "$value" > "${gpio_path}/value" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ 输出值设置为: ${value}${NC}"
        # 显示实际电平
        if [ "$value" = "1" ]; then
            echo -e "  电平: ${GREEN}高电平 (HIGH)${NC}"
        else
            echo -e "  电平: ${RED}低电平 (LOW)${NC}"
        fi
    else
        echo -e "${RED}错误: 无法设置值${NC}"
        exit 1
    fi
}

# 读取GPIO值
read_value() {
    local gpio=$1
    local gpio_path="${GPIO_BASE}/gpio${gpio}"
    
    if [ ! -d "$gpio_path" ]; then
        echo -e "${RED}错误: GPIO ${gpio} 未导出${NC}"
        echo "请先运行: $0 ${gpio} out 0 或 $0 ${gpio} in"
        exit 1
    fi
    
    local value=$(cat "${gpio_path}/value" 2>/dev/null)
    if [ $? -eq 0 ]; then
        echo -e "${BLUE}GPIO ${gpio} 当前值: ${value}${NC}"
        if [ "$value" = "1" ]; then
            echo -e "  电平: ${GREEN}高电平 (HIGH)${NC}"
        else
            echo -e "  电平: ${RED}低电平 (LOW)${NC}"
        fi
    else
        echo -e "${RED}错误: 无法读取GPIO值${NC}"
        exit 1
    fi
}

# 查看GPIO状态
show_status() {
    local gpio=$1
    local gpio_path="${GPIO_BASE}/gpio${gpio}"
    
    if [ ! -d "$gpio_path" ]; then
        echo -e "${RED}GPIO ${gpio} 未导出${NC}"
        return 1
    fi
    
    echo -e "${BLUE}=== GPIO ${gpio} 状态 ===${NC}"
    
    # 读取方向
    local direction=$(cat "${gpio_path}/direction" 2>/dev/null)
    if [ $? -eq 0 ]; then
        echo -e "方向: ${YELLOW}${direction}${NC}"
    fi
    
    # 读取值
    local value=$(cat "${gpio_path}/value" 2>/dev/null)
    if [ $? -eq 0 ]; then
        echo -e "当前值: ${YELLOW}${value}${NC}"
        if [ "$value" = "1" ]; then
            echo -e "电平: ${GREEN}高电平${NC}"
        else
            echo -e "电平: ${RED}低电平${NC}"
        fi
    fi
    
    # 读取边沿触发
    local edge=$(cat "${gpio_path}/edge" 2>/dev/null)
    if [ $? -eq 0 ] && [ -n "$edge" ]; then
        echo -e "边沿触发: ${YELLOW}${edge}${NC}"
    fi
    
    # 读取活动电平
    local active_low=$(cat "${gpio_path}/active_low" 2>/dev/null)
    if [ $? -eq 0 ]; then
        echo -e "活动电平: ${YELLOW}${active_low}${NC}"
    fi
}

# 带自动确认的取消导出
safe_unexport() {
    local gpio=$1
    echo -e "${YELLOW}确认取消导出GPIO ${gpio}? (y/n)${NC}"
    read -r answer
    if [ "$answer" = "y" ] || [ "$answer" = "Y" ]; then
        unexport_gpio "$gpio"
    else
        echo "操作取消"
    fi
}

# ============ 主程序 ============

# 检查参数
if [ $# -lt 2 ]; then
    usage
    exit 1
fi

GPIO_NUM=$1
ACTION=$2
VALUE=$3

# 检查root权限（除了help和usage）
check_root

# 执行操作
case "$ACTION" in
    out)
        if [ -z "$VALUE" ]; then
            echo -e "${RED}错误: 输出模式需要指定值 (0或1)${NC}"
            echo ""
            usage
            exit 1
        fi
        export_gpio "$GPIO_NUM"
        set_direction "$GPIO_NUM" "out"
        set_value "$GPIO_NUM" "$VALUE"
        ;;
        
    in)
        export_gpio "$GPIO_NUM"
        set_direction "$GPIO_NUM" "in"
        read_value "$GPIO_NUM"
        ;;
        
    read)
        read_value "$GPIO_NUM"
        ;;
        
    status)
        show_status "$GPIO_NUM"
        ;;
        
    unexport)
        unexport_gpio "$GPIO_NUM"
        ;;
        
    *)
        echo -e "${RED}错误: 未知操作 '${ACTION}'${NC}"
        echo ""
        usage
        exit 1
        ;;
esac

exit 0