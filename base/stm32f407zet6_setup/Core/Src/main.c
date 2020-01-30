/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define ILI9341_NOP            0x00
#define ILI9341_SWRESET        0x01

#define ILI9341_RDDID          0x04
#define ILI9341_RDDST          0x09
#define ILI9341_RDMODE         0x0A
#define ILI9341_RDMADCTL       0x0B
#define ILI9341_RDPIXFMT       0x0C
#define ILI9341_RDIMGFMT       0x0D
#define ILI9341_RDSELFDIAG     0x0F

#define ILI9341_SLPIN          0x10
#define ILI9341_SLPOUT         0x11
#define ILI9341_PTLON          0x12
#define ILI9341_NORON          0x13

#define ILI9341_INVOFF         0x20
#define ILI9341_INVON          0x21
#define ILI9341_GAMMASET       0x26
#define ILI9341_DISPOFF        0x28
#define ILI9341_DISPON         0x29

#define ILI9341_CASET          0x2A
#define ILI9341_PASET          0x2B
#define ILI9341_RAMWR          0x2C
#define ILI9341_RAMRD          0x2E

#define ILI9341_PTLAR          0x30
#define ILI9341_MADCTL         0x36
#define ILI9341_VSCRSADD       0x37     /* Vertical Scrolling Start Address */
#define ILI9341_PIXFMT         0x3A     /* COLMOD: Pixel Format Set */

#define ILI9341_RGB_INTERFACE  0xB0     /* RGB Interface Signal Control */
#define ILI9341_FRMCTR1        0xB1
#define ILI9341_FRMCTR2        0xB2
#define ILI9341_FRMCTR3        0xB3
#define ILI9341_INVCTR         0xB4
#define ILI9341_DFUNCTR        0xB6     /* Display Function Control */

#define ILI9341_PWCTR1         0xC0
#define ILI9341_PWCTR2         0xC1
#define ILI9341_PWCTR3         0xC2
#define ILI9341_PWCTR4         0xC3
#define ILI9341_PWCTR5         0xC4
#define ILI9341_VMCTR1         0xC5
#define ILI9341_VMCTR2         0xC7

#define ILI9341_RDID1          0xDA
#define ILI9341_RDID2          0xDB
#define ILI9341_RDID3          0xDC
#define ILI9341_RDID4          0xDD

#define ILI9341_GMCTRP1        0xE0
#define ILI9341_GMCTRN1        0xE1

#define ILI9341_PWCTR6         0xFC
#define ILI9341_INTERFACE      0xF6   /* Interface control register */

/* Extend register commands */
#define ILI9341_POWERA         0xCB   /* Power control A register */
#define ILI9341_POWERB         0xCF   /* Power control B register */
#define ILI9341_DTCA           0xE8   /* Driver timing control A */
#define ILI9341_DTCB           0xEA   /* Driver timing control B */
#define ILI9341_POWER_SEQ      0xED   /* Power on sequence register */
#define ILI9341_3GAMMA_EN      0xF2   /* 3 Gamma enable register */
#define ILI9341_PRC            0xF7   /* Pump ratio control register */

//-----------------------------------------------------------------------------
#define ILI9341_MAD_RGB        0x08
#define ILI9341_MAD_BGR        0x00

#define ILI9341_MAD_VERTICAL   0x20
#define ILI9341_MAD_X_LEFT     0x00
#define ILI9341_MAD_X_RIGHT    0x40
#define ILI9341_MAD_Y_UP       0x80
#define ILI9341_MAD_Y_DOWN     0x00

#define ILI9341_MAD_COLORMODE  ILI9341_MAD_RGB

#define ILI9341_SIZE_X                     ILI9341_LCD_PIXEL_WIDTH
#define ILI9341_SIZE_Y                     ILI9341_LCD_PIXEL_HEIGHT
#define ILI9341_MAD_DATA_RIGHT_THEN_UP     ILI9341_MAD_COLORMODE | ILI9341_MAD_X_RIGHT | ILI9341_MAD_Y_UP
#define ILI9341_MAD_DATA_RIGHT_THEN_DOWN   ILI9341_MAD_COLORMODE | ILI9341_MAD_X_RIGHT | ILI9341_MAD_Y_DOWN
#define ILI9341_MAD_DATA_RGBMODE           ILI9341_MAD_COLORMODE | ILI9341_MAD_X_LEFT | ILI9341_MAD_Y_DOWN
#define XPOS                               Xpos
#define YPOS                               Ypos
#define XSIZE                              Xsize
#define YSIZE                              Ysize
#define XSTEP                              1
#define YSTEP                              ILI9341_LCD_PIXEL_WIDTH

#define LCD_BASE_ADDRESS        0x6C000000
#define LCD_ADDRESS_6           (1 << (6 + 1))
// BANK-1 CS4 with FMC_ADDR6 high
#define LCD_DATA_ADDRESS        (LCD_BASE_ADDRESS | LCD_ADDRESS_6)
// BANK-1 CS4 with FMC_ADDR6 low
#define LCD_COMMAND_ADDRESS     (LCD_BASE_ADDRESS)

#define LCD_REG     ((volatile uint16_t*)LCD_COMMAND_ADDRESS)
#define LCD_DAT     ((volatile uint16_t*)LCD_DATA_ADDRESS)

static void
lcd_write_cmd8(uint8_t cmd)
{
  *LCD_REG = cmd;
}

static void
lcd_write_data8(uint8_t data)
{
  *LCD_DAT = data;
}

static void
lcd_write_data16(uint16_t data)
{
  *LCD_DAT = data;
}

static void
lcd_write_cmd_multiple(uint8_t cmd, uint8_t* data, uint32_t size)
{
  *LCD_REG = cmd;
  while(size--)
  {
    *LCD_DAT = *data;
    data++;
  }
}

#define LCD_IO_WriteData16_to_2x8(dt)    {lcd_write_data8((dt) >> 8); lcd_write_data8(dt); }

static void
lcd_write_pixel(uint16_t x, uint16_t y, uint16_t rgb)
{
  // set cursor
  lcd_write_cmd8(ILI9341_CASET);
  LCD_IO_WriteData16_to_2x8(x);
  LCD_IO_WriteData16_to_2x8(x);

  lcd_write_cmd8(ILI9341_PASET);
  LCD_IO_WriteData16_to_2x8(y);
  LCD_IO_WriteData16_to_2x8(y);

  // set pixel
  lcd_write_cmd8(ILI9341_RAMWR);
  lcd_write_data16(rgb);
}

static void
fill_lcd(void)
{
  uint16_t col, row;
  uint16_t colors[] = 
  {
    ((0x1f << 11) | (0x00 << 5) | 0x0),
    ((0x00 << 11) | (0x3f << 5) | 0x0),
    ((0x00 << 11) | (0x00 << 5) | 0x1f),
  };
  static uint8_t i;

  for(row = 0; row < 320; row++)
  {
    for(col = 0; col < 240; col++)
    {
      //
      // 5-6-5 format
      // 5 : 0x1F
      // 6 : 0x3F
      //
      lcd_write_pixel(col, row, colors[i % 3]);
    }
  }
  i++;
}

void
init_lcd(void)
{
  lcd_write_cmd8(ILI9341_SWRESET);
  HAL_Delay(10);

  lcd_write_cmd_multiple(0xEF, (uint8_t *)"\x03\x80\x02", 3);
  lcd_write_cmd_multiple(0xCF, (uint8_t *)"\x00\xC1\x30", 3);
  lcd_write_cmd_multiple(0xED, (uint8_t *)"\x64\x03\x12\x81", 4);
  lcd_write_cmd_multiple(0xE8, (uint8_t *)"\x85\x00\x78", 3);
  lcd_write_cmd_multiple(0xCB, (uint8_t *)"\x39\x2C\x00\x34\x02", 5);
  lcd_write_cmd_multiple(0xF7, (uint8_t *)"\x20", 1);
  lcd_write_cmd_multiple(0xEA, (uint8_t *)"\x00\x00", 2);

  // Power Control 1 (Vreg1out, Verg2out)
  lcd_write_cmd_multiple(ILI9341_PWCTR1, (uint8_t *)"\x23", 1);

  // Power Control 2 (VGH,VGL)
  lcd_write_cmd_multiple(ILI9341_PWCTR2, (uint8_t *)"\x10", 1);

  // Power Control 3 (Vcom)
  lcd_write_cmd_multiple(ILI9341_VMCTR1, (uint8_t *)"\x3E\x28", 2);

  // Power Control 3 (Vcom)
  lcd_write_cmd_multiple(ILI9341_VMCTR2, (uint8_t *)"\x86", 1);

  // Vertical scroll zero
  lcd_write_cmd_multiple(ILI9341_VSCRSADD, (uint8_t *)"\x00", 1);
  lcd_write_cmd_multiple(ILI9341_PIXFMT, (uint8_t *)"\x55", 1);

  // lcd_write_cmd_multiple(0xF6, (uint8_t *)"\x01\x00\x06", 3);

  lcd_write_cmd_multiple(ILI9341_FRMCTR1, (uint8_t *)"\x00\x18", 2);
  lcd_write_cmd_multiple(ILI9341_DFUNCTR, (uint8_t *)"\x08\x82\x27", 3);  // Display Function Control
  lcd_write_cmd_multiple(0xF2, (uint8_t *)"\x00", 1);            // 3Gamma Function Disable
  lcd_write_cmd_multiple(ILI9341_GAMMASET, (uint8_t *)"\x01", 1);// Gamma curve selected

  // positive gamma control
  lcd_write_cmd_multiple(ILI9341_GMCTRP1, (uint8_t *)"\x0F\x31\x2B\x0C\x0E\x08\x4E\xF1\x37\x07\x10\x03\x0E\x09\x00", 15);

  // negative gamma control
  lcd_write_cmd_multiple(ILI9341_GMCTRN1, (uint8_t *)"\x00\x0E\x14\x03\x11\x07\x31\xC1\x48\x08\x0F\x0C\x31\x36\x0F", 15);

  lcd_write_cmd8(ILI9341_MADCTL);
  lcd_write_data8(ILI9341_MAD_DATA_RIGHT_THEN_DOWN);

  lcd_write_cmd8(ILI9341_SLPOUT);    // Exit Sleep
  HAL_Delay(10);
  lcd_write_cmd8(ILI9341_DISPON);    // Display on
  HAL_Delay(10);
}

void
test_sram(void)
{
  #define BASER_ADDRESS           0x68000000    // Bank-1 CS3
  //
  // we have 1MB sram controlled by NE3
  //
  static volatile uint8_t*    base = (uint8_t*)BASER_ADDRESS;
  static volatile uint8_t*    current = (uint8_t*)BASER_ADDRESS;;
  const uint32_t              bytes_to_test = 1024 * 250;
  uint32_t                    i;
  uint8_t                     target;

  for (i = 0; i < bytes_to_test; i++)
  {
    if ((current - base) >= (1024 * 1024))
    {
      current = base;
    }

    target = (uint8_t)(i % 256);

    // write
    *current = target;

    // read back
    if (*current != target)
    {
      // error
      while(1)
      {
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        HAL_Delay(500);
      }
    }
    current++;
  }
  HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
}

void
test_sram_16(void)
{
  #define BASER_ADDRESS           0x68000000    // Bank-1 CS3
  //
  // we have 1MB sram controlled by NE3
  //
  static volatile uint16_t*   base = (uint16_t*)BASER_ADDRESS;
  static volatile uint16_t*   current = (uint16_t*)BASER_ADDRESS;;
  const uint32_t              bytes_to_test = 1024 * 250;
  uint32_t                    i;
  uint16_t                    target;

  for (i = 0; i < bytes_to_test; i++)
  {
    if ((current - base) >= (1024 * 1024))
    {
      current = base;
    }

    target = (uint16_t)(i % (0xffffU + 1));

    // write
    *current = target;

    // read back
    if (*current != target)
    {
      // error
      while(1)
      {
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        HAL_Delay(100);
      }
    }
    current++;
  }
  HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  init_lcd();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#if 0
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_Delay(50);
#else
    test_sram();
    test_sram_16();
    fill_lcd();
#endif
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
