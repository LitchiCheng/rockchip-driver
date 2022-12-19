## 介绍
1. rockchip saradc的测试程序
2. 使用saradc的第0个通道，ref电源1.8v，adc 10bit精度

## dts
在根节点下增加
```
adc_test: adc_test {
    compatible = "Litchi,adctest";
    io-channels = <&saradc 0>;
    io-channel-names = "saradc-test";
    status = "okay";
};
```

## make
修改KERNELDIR为内核目录:
KERNELDIR := your kernel path

## adctestapp.c
测试app，adc输出毫伏mv,输出如下
```
adc mv is 1800
adc mv is 1798
adc mv is 1800
adc mv is 1800
adc mv is 1798
adc mv is 1800
adc mv is 1800
adc mv is 1800
adc mv is 1800
```