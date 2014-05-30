# include <math.h>
# include "adc.h"

# define ADC_ROOMK_CHANNEL		1
# define ADC_PROBEK_CHANNEL		0

const float VOL_OFFSET = 350.0;		// offset voltage, mv
const float AMP_AV     = 54.16;		// Av of amplifier

// k voltage in mV http://www.ni.com/white-paper/4231/en/#toc2
// coeff polynomials for k thermocouplers in temp ranges -200 , 0 , 500 , 1372
const float Var_VtoT_K[3][10] =
{
    {0           , 2.5173462e1,   -1.1662878, -1.0833638   , -8.9773540/1e1, -3.7342377/1e1,-8.6632643/1e2 , -1.0450598/1e2 , -5.1920577/1e4                  },
    {0           , 2.508355e1 , 7.860106/1e2, -2.503131/1e1, 8.315270/1e2  , -1.228034/1e2 , 9.804036/1e4  , -4.413030/1e5  , 1.057734/1e6     , -1.052755/1e8},
    {-1.318058e2 , 4.830222e1 ,    -1.646031, 5.464731/1e2 , -9.650715/1e4 , 8.802193/1e6  , -3.110810/1e8                                                    }
};

// Beta 3975
// Res at 25C (298.15K) -> 10k (10000)
float getroomcelsius()
{
    const uint32_t vin = adc_filter_read(ADC_ROOMK_CHANNEL) * 50/33;       			// 3.3V supply
    const float resistance= (float)( (1023-vin)*10000 /vin ) ;
    const float temperature=1.0/(log(resistance/10000.0)/3975.0+1.0/298.15)-273.15; // convert to temperature via datasheet ;
    return temperature ;
}

float getprobevoltage()                     // get voltage of thmc in mV
{
	const float read = (float)adc_filter_read(ADC_PROBEK_CHANNEL) ;
    const float vout = read/1023.0*5.0*1000;
    const float vin  = (vout - VOL_OFFSET)/AMP_AV;
    return vin;
}

float K_VtoT(const float mV)
{
    uint8_t i = 0;
    float value = 0.0;

    // -200 - 0
    if(mV >= -6.478 && mV < 0)
    {
        value = Var_VtoT_K[0][8];

        for(i = 8; i >0; --i)
        {
        	value = mV * value + Var_VtoT_K[0][i-1];        	
        }
    }
    // 0 - 500
    else if(mV >= 0 && mV < 20.644)
    {
        value = Var_VtoT_K[1][9];

        for(i = 9; i >0; -- i)
        {
            value = mV * value + Var_VtoT_K[1][i-1];
        }
    }
    // 500 - 1372
    else if(mV >= 20.644 && mV <= 54.900)
    {
        value = Var_VtoT_K[2][6];

        for(i = 6; i >0; -- i)
        {
			value = mV * value + Var_VtoT_K[2][i-1];
        }
    }

    return value;
}

static float rtemp = 0.0;
static float ptemp = 0.0;
static float temp_rtemp = 0.0;
static float temp_ptemp = 0.0;
static uint8_t temp_test = 0;

# define MAXTEMPTEST    64

void temperature ( float * room , float * probe )
{
    temp_ptemp += K_VtoT( getprobevoltage() ) ;
    temp_rtemp += getroomcelsius() ;
    ++ temp_test ;

    if (temp_test > MAXTEMPTEST)
    {
        rtemp = temp_rtemp * (1.0/MAXTEMPTEST) ;
        ptemp = temp_ptemp * (1.0/MAXTEMPTEST) ;
        temp_rtemp = 0 ;
        temp_ptemp = 0 ;
        temp_test = 0 ;
    }

    * room = rtemp ;
    * probe= (rtemp + ptemp ) ;
}

