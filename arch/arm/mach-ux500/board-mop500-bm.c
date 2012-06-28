/*
 * Copyright (C) ST-Ericsson SA 2011
 *
 * License terms:  GNU General Public License (GPL), version 2
 *
 * U8500 board specific charger and battery initialization parameters.
 *
 * Author: Johan Palsson <johan.palsson@stericsson.com> for ST-Ericsson.
 * Author: Johan Gardsmark <johan.gardsmark@stericsson.com> for ST-Ericsson.
 *
 */

#include <linux/power_supply.h>
#include <linux/mfd/ab8500/ab8500-bm.h>
#include "board-mop500-bm.h"

#ifdef CONFIG_AB8500_BATTERY_THERM_ON_BATCTRL
/*
 * These are the defined batteries that uses a NTC and ID resistor placed
 * inside of the battery pack.
 * Note that the res_to_temp table must be strictly sorted by falling resistance
 * values to work.
 */
/* SEMC Type 1 battery */
static struct res_to_temp temp_tbl_A[] = {
	{-20, 67400},
	{  0, 49200},
	{  5, 44200},
	{ 10, 39400},
	{ 15, 35000},
	{ 20, 31000},
	{ 25, 27400},
	{ 30, 24300},
	{ 35, 21700},
	{ 40, 19400},
	{ 45, 17500},
	{ 50, 15900},
	{ 55, 14600},
	{ 60, 13500},
	{ 65, 12500},
	{ 70, 11800},
	{100,  9200},
};
/* SEMC Type 2 battery */
static struct res_to_temp temp_tbl_B[] = {
	{-20, 180700},
	{  0, 160000},
	{  5, 152700},
	{ 10, 144900},
	{ 15, 136800},
	{ 20, 128700},
	{ 25, 121000},
	{ 30, 113800},
	{ 35, 107300},
	{ 40, 101500},
	{ 45,  96500},
	{ 50,  92200},
	{ 55,  88600},
	{ 60,  85600},
	{ 65,  83000},
	{ 70,  80900},
	{100,  73900},
};
/* Discharge curve for 100 mA load for Lowe batteries at 23 degC */
static struct v_to_cap cap_tbl_A[] = {
	{4174,	100},
	{4135,	99},
	{4123,	98},
	{4112,	97},
	{4102,	96},
	{4092,	95},
	{4084,	94},
	{4074,	93},
	{4065,	92},
	{4056,	91},
	{4048,	90},
	{4041,	89},
	{4032,	88},
	{4023,	87},
	{4014,	86},
	{4005,	85},
	{3996,	84},
	{3988,	83},
	{3980,	82},
	{3971,	81},
	{3964,	80},
	{3957,	79},
	{3950,	78},
	{3943,	77},
	{3937,	76},
	{3930,	75},
	{3923,	74},
	{3918,	73},
	{3911,	72},
	{3904,	71},
	{3898,	70},
	{3892,	69},
	{3887,	68},
	{3880,	67},
	{3874,	66},
	{3868,	65},
	{3861,	64},
	{3856,	63},
	{3848,	62},
	{3841,	61},
	{3833,	60},
	{3825,	59},
	{3818,	58},
	{3812,	57},
	{3806,	56},
	{3801,	55},
	{3796,	54},
	{3792,	53},
	{3788,	52},
	{3783,	51},
	{3780,	50},
	{3776,	49},
	{3773,	48},
	{3770,	47},
	{3767,	46},
	{3764,	45},
	{3761,	44},
	{3758,	43},
	{3756,	42},
	{3753,	41},
	{3751,	40},
	{3748,	39},
	{3747,	38},
	{3744,	37},
	{3743,	36},
	{3741,	35},
	{3739,	34},
	{3737,	33},
	{3736,	32},
	{3733,	31},
	{3732,	30},
	{3729,	29},
	{3727,	28},
	{3725,	27},
	{3723,	26},
	{3721,	25},
	{3717,	24},
	{3713,	23},
	{3709,	22},
	{3706,	21},
	{3700,	20},
	{3695,	19},
	{3687,	18},
	{3679,	17},
	{3669,	16},
	{3661,	15},
	{3650,	14},
	{3645,	13},
	{3642,	12},
	{3639,	11},
	{3636,	10},
	{3632,	9},
	{3628,	8},
	{3622,	7},
	{3613,	6},
	{3601,	5},
	{3570,	4},
	{3519,	3},
	{3452,	2},
	{3366,	1},
	{3239,	0},
};


static struct v_to_cap cap_tbl_B[] = {
	{4161,	100},
	{4124,	 98},
	{4044,	 90},
	{4003,	 85},
	{3966,	 80},
	{3933,	 75},
	{3888,	 67},
	{3849,	 60},
	{3813,	 55},
	{3787,	 47},
	{3772,	 30},
	{3751,	 25},
	{3718,	 20},
	{3681,	 16},
	{3660,	 14},
	{3589,	 10},
	{3546,	  7},
	{3495,	  4},
	{3404,	  2},
	{3250,	  0},
};
#endif
static struct v_to_cap cap_tbl[] = {
	{4186,	100},
	{4163,	 99},
	{4114,	 95},
	{4068,	 90},
	{3990,	 80},
	{3926,	 70},
	{3898,	 65},
	{3866,	 60},
	{3833,	 55},
	{3812,	 50},
	{3787,	 40},
	{3768,	 30},
	{3747,	 25},
	{3730,	 20},
	{3705,	 15},
	{3699,	 14},
	{3684,	 12},
	{3672,	  9},
	{3657,	  7},
	{3638,	  6},
	{3556,	  4},
	{3424,	  2},
	{3317,	  1},
	{3094,	  0},
};

/*
 * Note that the res_to_temp table must be strictly sorted by falling
 * resistance values to work.
 */
static struct res_to_temp temp_tbl[] = {
	{-5, 214834},
	{ 0, 162943},
	{ 5, 124820},
	{10,  96520},
	{15,  75306},
	{20,  59254},
	{25,  47000},
	{30,  37566},
	{35,  30245},
	{40,  24520},
	{45,  20010},
	{50,  16432},
	{55,  13576},
	{60,  11280},
	{65,   9425},
};

#ifdef CONFIG_AB8500_BATTERY_THERM_ON_BATCTRL
/*
 * Note that the batres_vs_temp table must be strictly sorted by falling
 * temperature values to work.
 */
static struct batres_vs_temp temp_to_batres_tbl[] = {
	{ 40, 120},
	{ 30, 135},
	{ 20, 165},
	{ 10, 230},
	{ 00, 325},
	{-10, 445},
	{-20, 595},
};
#else
/*
 * Note that the batres_vs_temp table must be strictly sorted by falling
 * temperature values to work.
 */
static struct batres_vs_temp temp_to_batres_tbl[] = {
	{ 60, 300},
	{ 30, 300},
	{ 20, 300},
	{ 10, 300},
	{ 00, 300},
	{-10, 300},
	{-20, 300},
};
#endif

static const struct battery_type bat_type[] = {
	[BATTERY_UNKNOWN] = {
		/* First element always represent the UNKNOWN battery */
		.name = POWER_SUPPLY_TECHNOLOGY_UNKNOWN,
		.resis_high = 0,
		.resis_low = 0,
		.charge_full_design = 1500,
		.nominal_voltage = 3700,
		.termination_vol = 4050,
		.termination_curr = 200,
		.recharge_cap = 95,
		.normal_cur_lvl = 400,
		.normal_vol_lvl = 4100,
		.maint_a_cur_lvl = 400,
		.maint_a_vol_lvl = 4050,
		.maint_a_chg_timer_h = 60,
		.maint_b_cur_lvl = 400,
		.maint_b_vol_lvl = 4000,
		.maint_b_chg_timer_h = 200,
		.low_high_cur_lvl = 300,
		.low_high_vol_lvl = 4000,
		.n_temp_tbl_elements = ARRAY_SIZE(temp_tbl),
		.r_to_t_tbl = temp_tbl,
		.n_v_cap_tbl_elements = ARRAY_SIZE(cap_tbl),
		.v_to_cap_tbl = cap_tbl,
		.n_batres_tbl_elements = ARRAY_SIZE(temp_to_batres_tbl),
		.batres_tbl = temp_to_batres_tbl,
	},

#ifdef CONFIG_AB8500_BATTERY_THERM_ON_BATCTRL
	{
		.name = POWER_SUPPLY_TECHNOLOGY_LIPO,
		.resis_high = 70000,
		.resis_low = 8200,
		.nominal_voltage = 3700,
		.termination_vol = 4150,
		.recharge_cap = 95,
		.normal_vol_lvl = 4200,
		.maint_a_cur_lvl = 1050, /* 0.7C */
		.maint_a_vol_lvl = 4114, /* 95% */
		.maint_a_chg_timer_h = 60,
		.maint_b_cur_lvl = 1050, /* 0.7C */
		.maint_b_vol_lvl = 4100,
		.maint_b_chg_timer_h = 200,
		.low_high_cur_lvl = 400,
		.low_high_vol_lvl = 4000,
		.batt_res_offset = 150, /* +150mOhm to Battery resistance */
		.batt_vbat_offset = 8, /* +8mV to Battery voltage */
		.n_temp_tbl_elements = ARRAY_SIZE(temp_tbl_A),
		.r_to_t_tbl = temp_tbl_A,
		.n_v_cap_tbl_elements = ARRAY_SIZE(cap_tbl_A),
		.v_to_cap_tbl = cap_tbl_A,
		.n_batres_tbl_elements = ARRAY_SIZE(temp_to_batres_tbl),
		.batres_tbl = temp_to_batres_tbl,
	},
	{
		.name = POWER_SUPPLY_TECHNOLOGY_LiMn,
		.resis_high = 180000,
		.resis_low = 70001,
		.nominal_voltage = 3700,
		.termination_vol = 4150,
		.recharge_cap = 95,
		.normal_vol_lvl = 4200,
		.maint_a_cur_lvl = 1050, /* 0.7C */
		.maint_a_vol_lvl = 4094, /* 95% */
		.maint_a_chg_timer_h = 60,
		.maint_b_cur_lvl = 1050, /* 0.7C */
		.maint_b_vol_lvl = 4100,
		.maint_b_chg_timer_h = 200,
		.low_high_cur_lvl = 400,
		.low_high_vol_lvl = 4000,
		.n_temp_tbl_elements = ARRAY_SIZE(temp_tbl_B),
		.r_to_t_tbl = temp_tbl_B,
		.n_v_cap_tbl_elements = ARRAY_SIZE(cap_tbl_B),
		.v_to_cap_tbl = cap_tbl_B,
		.n_batres_tbl_elements = ARRAY_SIZE(temp_to_batres_tbl),
		.batres_tbl = temp_to_batres_tbl,
	},
#else
/*
 * These are the batteries that doesn't have an internal NTC resistor to measure
 * its temperature. The temperature in this case is measure with a NTC placed
 * near the battery but on the PCB.
 */
	{
		.name = POWER_SUPPLY_TECHNOLOGY_LIPO,
		.resis_high = 76000,
		.resis_low = 53000,
		.charge_full_design = 900,
		.nominal_voltage = 3700,
		.termination_vol = 4150,
		.termination_curr = 100,
		.recharge_cap = 95,
		.normal_vol_lvl = 4200,
		.maint_a_cur_lvl = 600,
		.maint_a_vol_lvl = 4114, /* 95% */
		.maint_a_chg_timer_h = 60,
		.maint_b_cur_lvl = 600,
		.maint_b_vol_lvl = 4100,
		.maint_b_chg_timer_h = 200,
		.low_high_cur_lvl = 300,
		.low_high_vol_lvl = 4000,
		.n_temp_tbl_elements = ARRAY_SIZE(temp_tbl),
		.r_to_t_tbl = temp_tbl,
		.n_v_cap_tbl_elements = ARRAY_SIZE(cap_tbl),
		.v_to_cap_tbl = cap_tbl,
		.n_batres_tbl_elements = ARRAY_SIZE(temp_to_batres_tbl),
		.batres_tbl = temp_to_batres_tbl,
	},
	{
		.name = POWER_SUPPLY_TECHNOLOGY_LION,
		.resis_high = 30000,
		.resis_low = 10000,
		.nominal_voltage = 3700,
		.termination_vol = 4150,
		.termination_curr = 100,
		.recharge_cap = 95,
		.normal_vol_lvl = 4200,
		.maint_a_cur_lvl = 600,
		.maint_a_vol_lvl = 4100,
		.maint_a_chg_timer_h = 60,
		.maint_b_cur_lvl = 600,
		.maint_b_vol_lvl = 4100,
		.maint_b_chg_timer_h = 200,
		.low_high_cur_lvl = 300,
		.low_high_vol_lvl = 4000,
		.n_temp_tbl_elements = ARRAY_SIZE(temp_tbl),
		.r_to_t_tbl = temp_tbl,
		.n_v_cap_tbl_elements = ARRAY_SIZE(cap_tbl),
		.v_to_cap_tbl = cap_tbl,
		.n_batres_tbl_elements = ARRAY_SIZE(temp_to_batres_tbl),
		.batres_tbl = temp_to_batres_tbl,
	},
	{
		.name = POWER_SUPPLY_TECHNOLOGY_LION,
		.resis_high = 95000,
		.resis_low = 76001,
		.nominal_voltage = 3700,
		.termination_vol = 4150,
		.termination_curr = 100,
		.recharge_cap = 95,
		.normal_cur_lvl = 700,
		.normal_vol_lvl = 4200,
		.maint_a_cur_lvl = 600,
		.maint_a_vol_lvl = 4100,
		.maint_a_chg_timer_h = 60,
		.maint_b_cur_lvl = 600,
		.maint_b_vol_lvl = 4100,
		.maint_b_chg_timer_h = 200,
		.low_high_cur_lvl = 300,
		.low_high_vol_lvl = 4000,
		.n_temp_tbl_elements = ARRAY_SIZE(temp_tbl),
		.r_to_t_tbl = temp_tbl,
		.n_v_cap_tbl_elements = ARRAY_SIZE(cap_tbl),
		.v_to_cap_tbl = cap_tbl,
		.n_batres_tbl_elements = ARRAY_SIZE(temp_to_batres_tbl),
		.batres_tbl = temp_to_batres_tbl,
	},
#endif
};

static char *ab8500_charger_supplied_to[] = {
	"ab8500_chargalg",
	"ab8500_fg",
	"ab8500_btemp",
};

static char *ab8500_btemp_supplied_to[] = {
	"ab8500_chargalg",
	"ab8500_fg",
};

static char *ab8500_fg_supplied_to[] = {
	"ab8500_chargalg",
	"ab8500_usb",
};

static char *ab8500_chargalg_supplied_to[] = {
	"ab8500_fg",
};

struct ab8500_charger_platform_data ab8500_charger_plat_data = {
	.supplied_to = ab8500_charger_supplied_to,
	.num_supplicants = ARRAY_SIZE(ab8500_charger_supplied_to),
};

struct ab8500_btemp_platform_data ab8500_btemp_plat_data = {
	.supplied_to = ab8500_btemp_supplied_to,
	.num_supplicants = ARRAY_SIZE(ab8500_btemp_supplied_to),
};

struct ab8500_fg_platform_data ab8500_fg_plat_data = {
	.supplied_to = ab8500_fg_supplied_to,
	.num_supplicants = ARRAY_SIZE(ab8500_fg_supplied_to),
	.ddata = &device_data,
};

struct ab8500_chargalg_platform_data ab8500_chargalg_plat_data = {
	.supplied_to = ab8500_chargalg_supplied_to,
	.num_supplicants = ARRAY_SIZE(ab8500_chargalg_supplied_to),
	.ddata = &device_data,
};

static const struct ab8500_bm_capacity_levels cap_levels = {
	.critical	= 2,
	.low		= 10,
	.normal		= 70,
	.high		= 95,
	.full		= 100,
};

static const struct ab8500_fg_parameters fg = {
	.recovery_sleep_timer = 10,
	.recovery_total_time = 100,
	.init_timer = 1,
	.init_discard_time = 5,
	.init_total_time = 40,
	.high_curr_time = 60,
	.accu_charging = 30,
	.accu_high_curr = 30,
	.high_curr_threshold = 125,
	.high_curr_exceed_thr = 60,
	.lowbat_threshold = 3250,
	.maint_thres = 95,
	.user_cap_limit = 15,
};

static const struct ab8500_maxim_parameters maxi_params = {
	.ena_maxi = true,
	.chg_curr = 1500,
	.wait_cycles = 10,
	.charger_curr_step = 100,
};

static const struct ab8500_bm_charger_parameters chg = {
	.usb_volt_max		= 5500,
	.usb_curr_max		= 1500,
	.usb_curr_max_nc	= 500,
	.ac_volt_max		= 7500,
	.ac_curr_max		= 1500,
};

struct ab8500_bm_data ab8500_bm_data = {
	.temp_under		= 5,
	.temp_low		= 5,
	.temp_high		= 45,
	.temp_over		= 55,
	.main_safety_tmr_h	= 4,
	.temp_interval_chg	= 20,
	.temp_interval_nochg	= 120,
	.usb_safety_tmr_h	= 24,
	.bkup_bat_v		= BUP_VCH_SEL_2P6V,
	.bkup_bat_i		= BUP_ICH_SEL_150UA,
	.no_maintenance		= true,
#ifdef CONFIG_AB8500_BATTERY_THERM_ON_BATCTRL
	.adc_therm		= ADC_THERM_BATCTRL,
#else
	.adc_therm		= ADC_THERM_BATTEMP,
#endif
	.chg_unknown_bat	= false,
	.enable_overshoot	= false,
	.fg_res			= 100,
	.cap_levels		= &cap_levels,
	.bat_type		= bat_type,
	.n_btypes		= ARRAY_SIZE(bat_type),
	.batt_id		= 0,
	.interval_charging	= 5,
	.interval_not_charging	= 120,
	.temp_hysteresis	= 3,
	.gnd_lift_resistance	= 34,
	.maxi			= &maxi_params,
	.chg_params		= &chg,
	.fg_params		= &fg,
};
