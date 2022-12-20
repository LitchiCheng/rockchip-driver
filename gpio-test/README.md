## 介绍
1. 测试gpio的输入输出

## dts
```
/ {
    gpio_test: gpio_test{
        compatible = "Litchi,gpiotest";
        test-gpio = <&gpio2 2 GPIO_ACTIVE_HIGH>;
        status = "okay";
    };
};
&pinctrl {
    gpio_test{
        gpio_test_pin: gpio_test_pin{
            rockchip,pins = <2 2 RK_FUNC_GPIO &pcfg_pull_none>;
        };
    };
};
```

## make
修改KERNELDIR为内核目录:
KERNELDIR := your kernel path

## gpiotestapp.c
1. 3秒钟反转一次GPIO2_2的输出
```