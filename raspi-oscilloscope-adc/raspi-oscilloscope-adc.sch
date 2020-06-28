EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L tlc5510:TLC5510INSR U2
U 1 1 5EF7D46D
P 6900 4000
F 0 "U2" H 6900 3300 50  0000 C CNN
F 1 "TLC5510INSR" H 6900 4700 50  0000 C CNN
F 2 "Package_DIP:DIP-24_W15.24mm" H 6800 3850 50  0001 C CNN
F 3 "" H 6800 3850 50  0001 C CNN
	1    6900 4000
	-1   0    0    1   
$EndComp
$Comp
L Oscillator:ECS-220B-100 U1
U 1 1 5EF81473
P 6900 2800
F 0 "U1" H 6900 3000 50  0000 C CNN
F 1 "ECS-220B-100" H 6900 2600 50  0000 C CNN
F 2 "Oscillator:Oscillator_DIP-8" H 6850 2750 50  0001 C CNN
F 3 "" H 6850 2750 50  0001 C CNN
	1    6900 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	7350 2750 7550 2750
Wire Wire Line
	7550 2750 7550 3450
Wire Wire Line
	7550 3450 7450 3450
$Comp
L power:GNDD #PWR0101
U 1 1 5EF841D0
P 7350 2850
F 0 "#PWR0101" H 7350 2600 50  0001 C CNN
F 1 "GNDD" H 7354 2695 50  0000 C CNN
F 2 "" H 7350 2850 50  0001 C CNN
F 3 "" H 7350 2850 50  0001 C CNN
	1    7350 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0102
U 1 1 5EF843FD
P 7450 4550
F 0 "#PWR0102" H 7450 4300 50  0001 C CNN
F 1 "GNDD" H 7454 4395 50  0000 C CNN
F 2 "" H 7450 4550 50  0001 C CNN
F 3 "" H 7450 4550 50  0001 C CNN
	1    7450 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 4550 7450 4450
Connection ~ 7450 4550
$Comp
L power:GNDD #PWR0103
U 1 1 5EF84A70
P 6350 4550
F 0 "#PWR0103" H 6350 4300 50  0001 C CNN
F 1 "GNDD" H 6354 4395 50  0000 C CNN
F 2 "" H 6350 4550 50  0001 C CNN
F 3 "" H 6350 4550 50  0001 C CNN
	1    6350 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 4150 6150 4150
Wire Wire Line
	6150 4150 6150 4250
Wire Wire Line
	6350 4250 6150 4250
Connection ~ 6150 4250
Wire Wire Line
	6150 4250 6150 4350
$Comp
L power:GNDA #PWR0104
U 1 1 5EF85117
P 6150 4450
F 0 "#PWR0104" H 6150 4200 50  0001 C CNN
F 1 "GNDA" H 6155 4277 50  0000 C CNN
F 2 "" H 6150 4450 50  0001 C CNN
F 3 "" H 6150 4450 50  0001 C CNN
	1    6150 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 4450 6150 4450
Wire Wire Line
	6350 4350 6150 4350
Connection ~ 6150 4350
Wire Wire Line
	6150 4350 6150 4450
Connection ~ 6150 4450
Wire Wire Line
	6350 3950 6300 3950
Wire Wire Line
	6300 3950 6300 3750
Wire Wire Line
	6350 3550 6300 3550
Wire Wire Line
	6350 3650 6300 3650
Connection ~ 6300 3650
Wire Wire Line
	6300 3650 6300 3550
Wire Wire Line
	6350 3750 6300 3750
Connection ~ 6300 3750
Wire Wire Line
	6300 3750 6300 3650
$Comp
L power:+5VD #PWR0105
U 1 1 5EF86C73
P 6350 3450
F 0 "#PWR0105" H 6350 3300 50  0001 C CNN
F 1 "+5VD" H 6365 3623 50  0000 C CNN
F 2 "" H 6350 3450 50  0001 C CNN
F 3 "" H 6350 3450 50  0001 C CNN
	1    6350 3450
	1    0    0    -1  
$EndComp
$Comp
L power:+5VD #PWR0106
U 1 1 5EF86F85
P 5950 2750
F 0 "#PWR0106" H 5950 2600 50  0001 C CNN
F 1 "+5VD" H 5965 2923 50  0000 C CNN
F 2 "" H 5950 2750 50  0001 C CNN
F 3 "" H 5950 2750 50  0001 C CNN
	1    5950 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6450 2850 6450 2750
Connection ~ 6450 2750
$Comp
L Amplifier_Operational:OPA690 U3
U 1 1 5EF8A6BD
P 4550 4400
F 0 "U3" H 4750 4650 50  0000 L CNN
F 1 "OPA690" H 4650 4550 50  0000 L CNN
F 2 "Package_DIP:DIP-8_W7.62mm" H 4550 4400 50  0001 C CNN
F 3 "" H 4550 4400 50  0001 C CNN
	1    4550 4400
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C3
U 1 1 5EF8DB0E
P 6200 2900
F 0 "C3" H 6292 2946 50  0000 L CNN
F 1 "0.1uF" H 6292 2855 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W5.0mm_P5.00mm" H 6200 2900 50  0001 C CNN
F 3 "~" H 6200 2900 50  0001 C CNN
	1    6200 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:CP1_Small C6
U 1 1 5EF8E1AB
P 5950 2900
F 0 "C6" H 5750 2950 50  0000 L CNN
F 1 "4.7uF" H 5650 2850 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 5950 2900 50  0001 C CNN
F 3 "~" H 5950 2900 50  0001 C CNN
	1    5950 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6450 2750 6200 2750
Wire Wire Line
	5950 2750 5950 2800
Wire Wire Line
	6200 2800 6200 2750
Connection ~ 6200 2750
Wire Wire Line
	6200 2750 5950 2750
Wire Wire Line
	6200 3000 5950 3000
$Comp
L power:GNDD #PWR013
U 1 1 5EF8FBEC
P 5950 3000
F 0 "#PWR013" H 5950 2750 50  0001 C CNN
F 1 "GNDD" H 5954 2845 50  0000 C CNN
F 2 "" H 5950 3000 50  0001 C CNN
F 3 "" H 5950 3000 50  0001 C CNN
	1    5950 3000
	1    0    0    -1  
$EndComp
Connection ~ 5950 3000
Connection ~ 5950 2750
$Comp
L Device:C_Small C2
U 1 1 5EF95A97
P 5850 3550
F 0 "C2" H 5942 3596 50  0000 L CNN
F 1 "0.1uF" H 5942 3505 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W5.0mm_P5.00mm" H 5850 3550 50  0001 C CNN
F 3 "~" H 5850 3550 50  0001 C CNN
	1    5850 3550
	1    0    0    -1  
$EndComp
$Comp
L Device:CP1_Small C5
U 1 1 5EF95A9D
P 5600 3550
F 0 "C5" H 5400 3600 50  0000 L CNN
F 1 "4.7uF" H 5300 3500 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 5600 3550 50  0001 C CNN
F 3 "~" H 5600 3550 50  0001 C CNN
	1    5600 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 3400 5600 3450
Wire Wire Line
	5850 3450 5850 3400
Wire Wire Line
	5850 3400 5600 3400
Wire Wire Line
	5850 3650 5600 3650
$Comp
L Device:R R7
U 1 1 5EF97081
P 5400 4050
F 0 "R7" V 5500 4050 50  0000 C CNN
F 1 "47" V 5600 4050 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 5330 4050 50  0001 C CNN
F 3 "~" H 5400 4050 50  0001 C CNN
	1    5400 4050
	0    1    1    0   
$EndComp
NoConn ~ 4700 3850
NoConn ~ 4600 4300
Wire Wire Line
	4600 3750 4500 3750
$Comp
L power:GNDA #PWR07
U 1 1 5EF9C4DE
P 4500 4350
F 0 "#PWR07" H 4500 4100 50  0001 C CNN
F 1 "GNDA" H 4350 4250 50  0000 C CNN
F 2 "" H 4500 4350 50  0001 C CNN
F 3 "" H 4500 4350 50  0001 C CNN
	1    4500 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 3850 5300 3050
Wire Wire Line
	5300 3050 5050 3050
$Comp
L Device:R R3
U 1 1 5EFA6C99
P 5050 3250
F 0 "R3" H 5120 3296 50  0000 L CNN
F 1 "330" H 5120 3205 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4980 3250 50  0001 C CNN
F 3 "~" H 5050 3250 50  0001 C CNN
	1    5050 3250
	1    0    0    -1  
$EndComp
$Comp
L power:GNDA #PWR09
U 1 1 5EFA7FB6
P 5050 3450
F 0 "#PWR09" H 5050 3200 50  0001 C CNN
F 1 "GNDA" H 5055 3277 50  0000 C CNN
F 2 "" H 5050 3450 50  0001 C CNN
F 3 "" H 5050 3450 50  0001 C CNN
	1    5050 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 3050 5050 3100
$Comp
L Device:R R2
U 1 1 5EFA8DEA
P 5050 2850
F 0 "R2" H 5120 2896 50  0000 L CNN
F 1 "82" H 5120 2805 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4980 2850 50  0001 C CNN
F 3 "~" H 5050 2850 50  0001 C CNN
	1    5050 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 3050 5050 3000
Connection ~ 5050 3050
$Comp
L power:+5VA #PWR08
U 1 1 5EFB5446
P 5050 2700
F 0 "#PWR08" H 5050 2550 50  0001 C CNN
F 1 "+5VA" H 5065 2873 50  0000 C CNN
F 2 "" H 5050 2700 50  0001 C CNN
F 3 "" H 5050 2700 50  0001 C CNN
	1    5050 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 4150 4200 4150
Wire Wire Line
	4200 4600 4400 4600
Wire Wire Line
	4200 4600 4200 4150
$Comp
L Device:R R1
U 1 1 5EFC7EB7
P 4200 4750
F 0 "R1" H 4270 4796 50  0000 L CNN
F 1 "470" H 4270 4705 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4130 4750 50  0001 C CNN
F 3 "~" H 4200 4750 50  0001 C CNN
	1    4200 4750
	1    0    0    -1  
$EndComp
Connection ~ 4200 4600
$Comp
L power:GNDA #PWR05
U 1 1 5EFC8309
P 4200 4900
F 0 "#PWR05" H 4200 4650 50  0001 C CNN
F 1 "GNDA" H 4205 4727 50  0000 C CNN
F 2 "" H 4200 4900 50  0001 C CNN
F 3 "" H 4200 4900 50  0001 C CNN
	1    4200 4900
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5EFC86EB
P 4150 3950
F 0 "R6" V 3943 3950 50  0000 C CNN
F 1 "47" V 4034 3950 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4080 3950 50  0001 C CNN
F 3 "~" H 4150 3950 50  0001 C CNN
	1    4150 3950
	0    1    1    0   
$EndComp
Connection ~ 4500 3750
Wire Wire Line
	4500 3750 4500 3700
$Comp
L power:+5VA #PWR06
U 1 1 5EFC9D1E
P 4500 3700
F 0 "#PWR06" H 4500 3550 50  0001 C CNN
F 1 "+5VA" H 4515 3873 50  0000 C CNN
F 2 "" H 4500 3700 50  0001 C CNN
F 3 "" H 4500 3700 50  0001 C CNN
	1    4500 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 3950 3850 3950
Wire Wire Line
	3850 3950 3850 3750
$Comp
L Device:R R4
U 1 1 5EFCB27C
P 3850 3600
F 0 "R4" H 3920 3646 50  0000 L CNN
F 1 "1k5" H 3920 3555 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3780 3600 50  0001 C CNN
F 3 "~" H 3850 3600 50  0001 C CNN
	1    3850 3600
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5EFCB749
P 3850 4300
F 0 "R5" H 3920 4346 50  0000 L CNN
F 1 "1k" H 3920 4255 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 3780 4300 50  0001 C CNN
F 3 "~" H 3850 4300 50  0001 C CNN
	1    3850 4300
	1    0    0    -1  
$EndComp
$Comp
L power:GNDA #PWR04
U 1 1 5EFCBCEF
P 3850 4450
F 0 "#PWR04" H 3850 4200 50  0001 C CNN
F 1 "GNDA" H 3855 4277 50  0000 C CNN
F 2 "" H 3850 4450 50  0001 C CNN
F 3 "" H 3850 4450 50  0001 C CNN
	1    3850 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3850 4150 3850 3950
Connection ~ 3850 3950
$Comp
L Device:C_Small C4
U 1 1 5EFD2C4C
P 4850 3250
F 0 "C4" H 4700 3300 50  0000 L CNN
F 1 "0.1uF" H 4600 3150 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W5.0mm_P5.00mm" H 4850 3250 50  0001 C CNN
F 3 "~" H 4850 3250 50  0001 C CNN
	1    4850 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:CP1_Small C7
U 1 1 5EFD2C52
P 4550 3250
F 0 "C7" H 4400 3300 50  0000 L CNN
F 1 "4.7uF" H 4300 3150 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 4550 3250 50  0001 C CNN
F 3 "~" H 4550 3250 50  0001 C CNN
	1    4550 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 5EFE4DC7
P 3250 3950
F 0 "C1" V 3100 3900 50  0000 L CNN
F 1 "0.1uF" V 3400 3850 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W5.0mm_P5.00mm" H 3250 3950 50  0001 C CNN
F 3 "~" H 3250 3950 50  0001 C CNN
	1    3250 3950
	0    1    1    0   
$EndComp
Wire Wire Line
	3850 3950 3500 3950
$Comp
L Device:D_Zener_Small D2
U 1 1 5EFEA21D
P 3500 4300
F 0 "D2" V 3454 4370 50  0000 L CNN
F 1 "5V" V 3545 4370 50  0000 L CNN
F 2 "Diode_THT:D_A-405_P7.62mm_Horizontal" V 3500 4300 50  0001 C CNN
F 3 "~" V 3500 4300 50  0001 C CNN
	1    3500 4300
	0    1    1    0   
$EndComp
Wire Wire Line
	3500 4200 3500 3950
Connection ~ 3500 3950
Wire Wire Line
	3500 3950 3350 3950
Wire Wire Line
	3500 4400 3500 4450
$Comp
L power:GNDA #PWR02
U 1 1 5EFED644
P 3500 4450
F 0 "#PWR02" H 3500 4200 50  0001 C CNN
F 1 "GNDA" H 3505 4277 50  0000 C CNN
F 2 "" H 3500 4450 50  0001 C CNN
F 3 "" H 3500 4450 50  0001 C CNN
	1    3500 4450
	1    0    0    -1  
$EndComp
$Comp
L power:+5VA #PWR03
U 1 1 5EFEE133
P 3850 3450
F 0 "#PWR03" H 3850 3300 50  0001 C CNN
F 1 "+5VA" H 3865 3623 50  0000 C CNN
F 2 "" H 3850 3450 50  0001 C CNN
F 3 "" H 3850 3450 50  0001 C CNN
	1    3850 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 3050 4550 3150
Wire Wire Line
	4550 3050 4850 3050
Wire Wire Line
	4850 3150 4850 3050
Connection ~ 4850 3050
Wire Wire Line
	4850 3050 5050 3050
Wire Wire Line
	4550 3450 4850 3450
Wire Wire Line
	4550 3350 4550 3450
Wire Wire Line
	5050 3400 5050 3450
Connection ~ 5050 3450
Wire Wire Line
	4850 3350 4850 3450
Connection ~ 4850 3450
Wire Wire Line
	4850 3450 5050 3450
$Comp
L Device:D_Zener_Small D1
U 1 1 5EFFEC87
P 5750 4300
F 0 "D1" V 5704 4370 50  0000 L CNN
F 1 "4V" V 5795 4370 50  0000 L CNN
F 2 "Diode_THT:D_A-405_P7.62mm_Horizontal" V 5750 4300 50  0001 C CNN
F 3 "~" V 5750 4300 50  0001 C CNN
	1    5750 4300
	0    1    1    0   
$EndComp
Wire Wire Line
	5750 4200 5750 4050
Wire Wire Line
	5550 4050 5750 4050
Wire Wire Line
	5750 4050 6350 4050
Connection ~ 5750 4050
Wire Wire Line
	5750 4400 5750 4450
$Comp
L power:GNDA #PWR012
U 1 1 5F003F73
P 5750 4450
F 0 "#PWR012" H 5750 4200 50  0001 C CNN
F 1 "GNDA" H 5755 4277 50  0000 C CNN
F 2 "" H 5750 4450 50  0001 C CNN
F 3 "" H 5750 4450 50  0001 C CNN
	1    5750 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 3550 7700 3550
Wire Wire Line
	7700 3550 7700 3350
$Comp
L power:+5VD #PWR014
U 1 1 5F00AC42
P 7700 3350
F 0 "#PWR014" H 7700 3200 50  0001 C CNN
F 1 "+5VD" H 7715 3523 50  0000 C CNN
F 2 "" H 7700 3350 50  0001 C CNN
F 3 "" H 7700 3350 50  0001 C CNN
	1    7700 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	8000 3650 7450 3650
Wire Wire Line
	7450 3750 8000 3750
Wire Wire Line
	8000 3850 7450 3850
Wire Wire Line
	7450 3950 8000 3950
Wire Wire Line
	8000 4050 7450 4050
Wire Wire Line
	7450 4150 8000 4150
Wire Wire Line
	7450 4250 8000 4250
Wire Wire Line
	7450 4350 8000 4350
$Comp
L Connector:Conn_Coaxial J1
U 1 1 5F01FB3A
P 2950 3950
F 0 "J1" H 2950 4200 50  0000 C CNN
F 1 "Conn_Coaxial" H 3000 4100 50  0000 C CNN
F 2 "Connector_Coaxial:1-1337543-0" H 2950 3950 50  0001 C CNN
F 3 " ~" H 2950 3950 50  0001 C CNN
	1    2950 3950
	-1   0    0    -1  
$EndComp
$Comp
L power:GNDA #PWR01
U 1 1 5F028229
P 2950 4150
F 0 "#PWR01" H 2950 3900 50  0001 C CNN
F 1 "GNDA" H 2955 3977 50  0000 C CNN
F 2 "" H 2950 4150 50  0001 C CNN
F 3 "" H 2950 4150 50  0001 C CNN
	1    2950 4150
	1    0    0    -1  
$EndComp
$Comp
L power:+5VA #PWR0107
U 1 1 5EFB6CFD
P 5600 3400
F 0 "#PWR0107" H 5600 3250 50  0001 C CNN
F 1 "+5VA" H 5600 3550 50  0000 C CNN
F 2 "" H 5600 3400 50  0001 C CNN
F 3 "" H 5600 3400 50  0001 C CNN
	1    5600 3400
	1    0    0    -1  
$EndComp
Connection ~ 5600 3400
Wire Wire Line
	5300 3850 6350 3850
Connection ~ 5850 3400
$Comp
L power:GNDA #PWR0108
U 1 1 5EFCB4C5
P 5600 3650
F 0 "#PWR0108" H 5600 3400 50  0001 C CNN
F 1 "GNDA" H 5750 3550 50  0000 C CNN
F 2 "" H 5600 3650 50  0001 C CNN
F 3 "" H 5600 3650 50  0001 C CNN
	1    5600 3650
	1    0    0    -1  
$EndComp
Connection ~ 5600 3650
Wire Wire Line
	6200 3400 6200 3550
Wire Wire Line
	6200 3550 6300 3550
Wire Wire Line
	5850 3400 6200 3400
Connection ~ 6300 3550
$Comp
L Connector:Conn_01x10_Male J2
U 1 1 5EFDEB4A
P 8200 4050
F 0 "J2" H 8172 3932 50  0000 R CNN
F 1 "Conn_01x10_Male" H 8172 4023 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x10_P2.54mm_Vertical" H 8200 4050 50  0001 C CNN
F 3 "~" H 8200 4050 50  0001 C CNN
	1    8200 4050
	-1   0    0    1   
$EndComp
Wire Wire Line
	7450 4450 8000 4450
Connection ~ 7450 4450
Wire Wire Line
	8000 3550 7700 3550
Connection ~ 7700 3550
$Comp
L Switch:SW_SP6T SW1
U 1 1 5F02B9D1
P 4600 4750
F 0 "SW1" H 4550 4750 50  0000 C CNN
F 1 "SR10010F-0106-20F0B-C7-N-0027" H 5200 4550 50  0000 C CNN
F 2 "Button_Switch_THT:SW_SP6T" H 4600 4750 50  0001 C CNN
F 3 "" H 4600 4750 50  0001 C CNN
	1    4600 4750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R8
U 1 1 5F02E03B
P 4950 4450
F 0 "R8" V 4950 4450 50  0000 C CNN
F 1 "4k7" V 4950 4700 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4880 4450 50  0001 C CNN
F 3 "~" H 4950 4450 50  0001 C CNN
	1    4950 4450
	0    1    1    0   
$EndComp
$Comp
L Device:R R9
U 1 1 5F02E4EA
P 4950 4550
F 0 "R9" V 4950 4550 50  0000 C CNN
F 1 "47k" V 4950 4800 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4880 4550 50  0001 C CNN
F 3 "~" H 4950 4550 50  0001 C CNN
	1    4950 4550
	0    1    1    0   
$EndComp
Wire Wire Line
	4900 4050 5100 4050
Wire Wire Line
	5100 4550 5100 4450
Connection ~ 5100 4050
Wire Wire Line
	5100 4050 5250 4050
Connection ~ 5100 4450
Wire Wire Line
	5100 4050 5100 4350
Wire Wire Line
	4800 4350 5100 4350
Connection ~ 5100 4350
Wire Wire Line
	5100 4350 5100 4450
$Comp
L Connector:Conn_01x02_Male J3
U 1 1 5F04F3F5
P 5400 2250
F 0 "J3" V 5462 2062 50  0000 R CNN
F 1 "Conn_01x02_Male" V 5553 2062 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 5400 2250 50  0001 C CNN
F 3 "~" H 5400 2250 50  0001 C CNN
	1    5400 2250
	0    -1   1    0   
$EndComp
$Comp
L power:GNDA #PWR0109
U 1 1 5F05489D
P 5500 2450
F 0 "#PWR0109" H 5500 2200 50  0001 C CNN
F 1 "GNDA" H 5505 2277 50  0000 C CNN
F 2 "" H 5500 2450 50  0001 C CNN
F 3 "" H 5500 2450 50  0001 C CNN
	1    5500 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 2450 5250 2450
$Comp
L power:+5VA #PWR0110
U 1 1 5F05AB98
P 5250 2450
F 0 "#PWR0110" H 5250 2300 50  0001 C CNN
F 1 "+5VA" H 5265 2623 50  0000 C CNN
F 2 "" H 5250 2450 50  0001 C CNN
F 3 "" H 5250 2450 50  0001 C CNN
	1    5250 2450
	1    0    0    -1  
$EndComp
NoConn ~ 4800 4650
NoConn ~ 4800 4750
NoConn ~ 4800 4850
$EndSCHEMATC
