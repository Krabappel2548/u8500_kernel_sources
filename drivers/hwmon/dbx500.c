/*
 * Copyright (C) ST-Ericsson SA 2010. All rights reserved.
 * This code is ST-Ericsson proprietary and confidential.
 * Any use of the code for whatever purpose is subject to
 * specific written permission of ST-Ericsson SA.
 *
 * Author: WenHai Fang <wenhai.h.fang@stericsson.com> for
 * ST-Ericsson.
 * License terms: GNU Gereral Public License (GPL) version 2
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <mach/prcmu.h>
#include <linux/hwmon.h>
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <mach/hardware.h>

/*
 * Default measure period to 0xFF x cycle32k
 */
#define DEFAULT_MEASURE_TIME 0xFF

/*
 * Default critical sensor temperature
 */
#define DEFAULT_CRITICAL_TEMP 85

/* This driver monitors DB thermal*/
#define NUM_SENSORS 1

struct dbx500_temp {
	struct platform_device *pdev;
	struct device *hwmon_dev;
	unsigned char min[NUM_SENSORS];
	unsigned char max[NUM_SENSORS];
	unsigned char crit[NUM_SENSORS];
	unsigned char min_alarm[NUM_SENSORS];
	unsigned char max_alarm[NUM_SENSORS];
	unsigned short measure_time;
	bool monitoring_active;
	struct mutex lock;
};

static inline void start_temp_monitoring(struct dbx500_temp *data,
					 const int index)
{
	unsigned int i;

	/* determine if there are any sensors worth monitoring */
	for (i = 0; i < NUM_SENSORS; i++)
		if (data->min[i] || data->max[i])
			goto start_monitoring;

	return;

start_monitoring:
	/* kick off the monitor job */
	data->min_alarm[index] = 0;
	data->max_alarm[index] = 0;

	(void) prcmu_start_temp_sense(data->measure_time);
	data->monitoring_active = true;
}

static inline void stop_temp_monitoring(struct dbx500_temp *data)
{
	if (data->monitoring_active) {
		(void) prcmu_stop_temp_sense();
		data->monitoring_active = false;
	}
}

/* HWMON sysfs interface */
static ssize_t show_name(struct device *dev, struct device_attribute *devattr,
			 char *buf)
{
	return sprintf(buf, "dbx500\n");
}

static ssize_t show_label(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	return show_name(dev, devattr, buf);
}

/* set functions (RW nodes) */
static ssize_t set_min(struct device *dev, struct device_attribute *devattr,
		       const char *buf, size_t count)
{
	unsigned long val;
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	int res = strict_strtoul(buf, 10, &val);
	if (res < 0)
		return res;

	mutex_lock(&data->lock);
	val &= 0xFF;
	if (val > data->max[attr->index - 1])
		val = data->max[attr->index - 1];

	data->min[attr->index - 1] = val;

	stop_temp_monitoring(data);

	(void) prcmu_config_hotmon(data->min[attr->index - 1],
			data->max[attr->index - 1]);

	start_temp_monitoring(data, (attr->index - 1));

	mutex_unlock(&data->lock);
	return count;
}

static ssize_t set_max(struct device *dev, struct device_attribute *devattr,
		       const char *buf, size_t count)
{
	unsigned long val;
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	int res = strict_strtoul(buf, 10, &val);
	if (res < 0)
		return res;

	mutex_lock(&data->lock);
	val &= 0xFF;
	if (val < data->min[attr->index - 1])
		val = data->min[attr->index - 1];

	data->max[attr->index - 1] = val;

	stop_temp_monitoring(data);

	(void) prcmu_config_hotmon(data->min[attr->index - 1],
		data->max[attr->index - 1]);

	start_temp_monitoring(data, (attr->index - 1));

	mutex_unlock(&data->lock);

	return count;
}

static ssize_t set_crit(struct device *dev,
			    struct device_attribute *devattr,
			    const char *buf, size_t count)
{
	unsigned long val;
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	int res = strict_strtoul(buf, 10, &val);
	if (res < 0)
		return res;

	mutex_lock(&data->lock);
	val &= 0xFF;
	data->crit[attr->index - 1] = val;
	(void) prcmu_config_hotdog(data->crit[attr->index - 1]);
	mutex_unlock(&data->lock);

	return count;
}

/*
 * show functions (RO nodes)
 * Notice that min/max/crit refer to degrees
 */
static ssize_t show_min(struct device *dev,
			struct device_attribute *devattr, char *buf)
{
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	/* hwmon attr index starts at 1, thus "attr->index-1" below */
	return sprintf(buf, "%d\n", data->min[attr->index - 1]);
}

static ssize_t show_max(struct device *dev,
			struct device_attribute *devattr, char *buf)
{
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	/* hwmon attr index starts at 1, thus "attr->index-1" below */
	return sprintf(buf, "%d\n", data->max[attr->index - 1]);
}

static ssize_t show_crit(struct device *dev,
			     struct device_attribute *devattr, char *buf)
{
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	/* hwmon attr index starts at 1, thus "attr->index-1" below */
	return sprintf(buf, "%d\n", data->crit[attr->index - 1]);
}

/* Alarms */
static ssize_t show_min_alarm(struct device *dev,
			      struct device_attribute *devattr, char *buf)
{
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	/* hwmon attr index starts at 1, thus "attr->index-1" below */
	return sprintf(buf, "%d\n", data->min_alarm[attr->index - 1]);
}

static ssize_t show_max_alarm(struct device *dev,
			      struct device_attribute *devattr, char *buf)
{
	struct dbx500_temp *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	/* hwmon attr index starts at 1, thus "attr->index-1" below */
	return sprintf(buf, "%d\n", data->max_alarm[attr->index - 1]);
}

/* Chip name, required by hwmon*/
static SENSOR_DEVICE_ATTR(name, S_IRUGO, show_name, NULL, 0);
static SENSOR_DEVICE_ATTR(temp1_min, S_IWUSR | S_IRUGO, show_min, set_min, 1);
static SENSOR_DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO, show_max, set_max, 1);
static SENSOR_DEVICE_ATTR(temp1_crit, S_IWUSR | S_IRUGO,
			show_crit, set_crit, 1);
static SENSOR_DEVICE_ATTR(temp1_label, S_IRUGO, show_label, NULL, 1);
static SENSOR_DEVICE_ATTR(temp1_min_alarm, S_IRUGO, show_min_alarm, NULL, 1);
static SENSOR_DEVICE_ATTR(temp1_max_alarm, S_IRUGO, show_max_alarm, NULL, 1);

static struct attribute *dbx500_temp_attributes[] = {
	&sensor_dev_attr_name.dev_attr.attr,
	&sensor_dev_attr_temp1_min.dev_attr.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp1_crit.dev_attr.attr,
	&sensor_dev_attr_temp1_label.dev_attr.attr,
	&sensor_dev_attr_temp1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_max_alarm.dev_attr.attr,
	NULL
};

static const struct attribute_group dbx500_temp_group = {
	.attrs = dbx500_temp_attributes,
};

static irqreturn_t prcmu_hotmon_low_irq_handler(int irq, void *irq_data)
{
	struct platform_device *pdev = irq_data;
	struct dbx500_temp *data = platform_get_drvdata(pdev);

	mutex_lock(&data->lock);
	data->min_alarm[0] = 1;
	mutex_unlock(&data->lock);

	sysfs_notify(&pdev->dev.kobj, NULL, "temp1_min_alarm");
	dev_dbg(&pdev->dev, "DBX500 thermal low warning\n");
	return IRQ_HANDLED;
}

static irqreturn_t prcmu_hotmon_high_irq_handler(int irq, void *irq_data)
{
	struct platform_device *pdev = irq_data;
	struct dbx500_temp *data = platform_get_drvdata(pdev);

	mutex_lock(&data->lock);
	data->max_alarm[0] = 1;
	mutex_unlock(&data->lock);

	hwmon_notify(data->max_alarm[0], NULL);
	sysfs_notify(&pdev->dev.kobj, NULL, "temp1_max_alarm");

	return IRQ_HANDLED;
}

static int __devinit dbx500_temp_probe(struct platform_device *pdev)
{
	struct dbx500_temp *data;
	int err = 0, i;
	int irq;

	dev_dbg(&pdev->dev, "dbx500_temp: Function dbx500_temp_probe.\n");

	data = kzalloc(sizeof(struct dbx500_temp), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	irq = platform_get_irq_byname(pdev, "IRQ_HOTMON_LOW");
	if (irq < 0) {
		dev_err(&pdev->dev, "Get IRQ_HOTMON_LOW failed\n");
		goto exit;
	}

	err = request_threaded_irq(irq, NULL,
				prcmu_hotmon_low_irq_handler,
				IRQF_NO_SUSPEND,
				"dbx500_temp_low", pdev);
	if (err < 0) {
		dev_err(&pdev->dev, "dbx500: Failed allocate HOTMON_LOW.\n");
		goto exit;
	} else {
		dev_dbg(&pdev->dev, "dbx500: Succeed allocate HOTMON_LOW.\n");
	}

	irq = platform_get_irq_byname(pdev, "IRQ_HOTMON_HIGH");
	if (irq < 0) {
		dev_err(&pdev->dev, "Get IRQ_HOTMON_HIGH failed\n");
		goto exit;
	}

	err = request_threaded_irq(irq, NULL,
				prcmu_hotmon_high_irq_handler,
				IRQF_NO_SUSPEND,
				 "dbx500_temp_high", pdev);
	if (err < 0) {
		dev_err(&pdev->dev, "dbx500: Failed allocate HOTMON_HIGH.\n");
		goto exit;
	} else {
		dev_dbg(&pdev->dev, "dbx500: Succeed allocate HOTMON_HIGH.\n");
	}

	data->hwmon_dev = hwmon_device_register(&pdev->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		dev_err(&pdev->dev, "Class registration failed (%d)\n", err);
		goto exit;
	}

	for (i = 0; i < NUM_SENSORS; i++) {
		data->min[i] = 0;
		data->max[i] = 0;
		data->crit[i] = DEFAULT_CRITICAL_TEMP;
		data->min_alarm[i] = 0;
		data->max_alarm[i] = 0;
	}

	mutex_init(&data->lock);

	data->pdev = pdev;
	data->measure_time = DEFAULT_MEASURE_TIME;
	data->monitoring_active = false;

	/* set PRCMU to disable platform when we get to the critical temp */
	(void) prcmu_config_hotdog(DEFAULT_CRITICAL_TEMP);

	platform_set_drvdata(pdev, data);

	err = sysfs_create_group(&pdev->dev.kobj, &dbx500_temp_group);
	if (err < 0) {
		dev_err(&pdev->dev, "Create sysfs group failed (%d)\n", err);
		goto exit_platform_data;
	}

	return 0;

exit_platform_data:
	platform_set_drvdata(pdev, NULL);
exit:
	kfree(data);
	return err;
}

static int __devexit dbx500_temp_remove(struct platform_device *pdev)
{
	struct dbx500_temp *data = platform_get_drvdata(pdev);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&pdev->dev.kobj, &dbx500_temp_group);
	platform_set_drvdata(pdev, NULL);
	kfree(data);
	return 0;
}

/* No action required in suspend/resume, thus the lack of functions */
static struct platform_driver dbx500_temp_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "dbx500_temp",
	},
	.probe = dbx500_temp_probe,
	.remove = __devexit_p(dbx500_temp_remove),
};

static int __init dbx500_temp_init(void)
{
	return platform_driver_register(&dbx500_temp_driver);
}

static void __exit dbx500_temp_exit(void)
{
	platform_driver_unregister(&dbx500_temp_driver);
}

MODULE_AUTHOR("WenHai Fang <wenhai.h.fang@stericsson.com>");
MODULE_DESCRIPTION("DBX500 temperature driver");
MODULE_LICENSE("GPL");

module_init(dbx500_temp_init)
module_exit(dbx500_temp_exit)
