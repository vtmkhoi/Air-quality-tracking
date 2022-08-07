#include "SoftwareSerial.h"
#include <WireData.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <TimeLib.h>
const int slaveAddress = 0x08;

#define MQ7      A0
#define SO2      A1
#define O3       A2
#define NO2      A3

SoftwareSerial mySerial(11, 10);

RTC_DS1307 RTC;


// Khai bao bien
unsigned int PM2_5,PM10=0;

float nowcast,nowcast2_5,nowcast10,tb_maxo3;
float aqi_pm2_5,aqi_pm10,aqi_co,aqi_so2,aqi_o3,aqi_no2,AQI_max;
int   aqi_h_pm2_5,aqi_h_pm10,aqi_h_co,aqi_h_so2,aqi_h_o3,aqi_h_no2,aqi_d_pm2_5,aqi_d_pm10,aqi_d_co,aqi_d_so2,aqi_d_o3,aqi_d_no2,AQI_h_max,AQI_d_max;

float Ratio_co,vRl_1,val_1,mq7Val,co =0;
float Ratio_so2,vRl_so2,val_so2,so2Val,so2=0;
float Ratio_o3,vRl_o3,val_o3,o3Val,o3  =0;
float Ratio_4,vRl_4,val_4,no2Val,no2=0;

float R0_co  =1907;
float Rl_co  =1000;

float R0_so2 =3431;
float Rl_so2 =1000;

float R0_o3  =170118;
float Rl_o3  =1000;

float R0_no2  =508329;
float Rl_no2  =47000;

int x,y,z;
int i=0;

//Khai bao mang
//Mang SDS011
unsigned char buf[7], buffSDS[25];
//Mang RTC
char daysOfTheWeek[7][12];
//Trung bình 1h(5p lấy mẫu 1 lần)
float mang_TB_1h[6][13];
//Trung bình 24h(1h lưu mẫu 1 lần)
float mang_TB_24h[7][25];
//Mang AQI gio(1h lưu mẫu 1 lần)
int mang_AQI_h[7][24];
//Mang AQI thang(1 day lưu mẫu 1 lần)
int mang_AQI_t[7][31];
//Mang nowcast
float mang_nowcast[12];
//Mang AQI
float mang_AQI[6];


void setup()
{
  // Serial Monitor
  Serial.begin(9600); 
  //Truyền I2C
  Wire.begin();
  RTC.begin();
  // Ket noi sensor MQ07,MQ136,MQ131
  pinMode(MQ7   ,INPUT);
  pinMode(SO2   ,INPUT);
  pinMode(O3    ,INPUT);
  pinMode(NO2   ,INPUT);
  pinMode(7     ,OUTPUT);
  pinMode(6     ,OUTPUT);
  pinMode(5     ,OUTPUT);
  pinMode(4     ,OUTPUT);
  pinMode(3     ,OUTPUT);
  pinMode(2     ,OUTPUT);
  // Read SDS011 on Serial 
  mySerial.begin(9600);
  mySerial.setTimeout(500);
  mySerial.readBytesUntil(0xAB,buffSDS,20); // read serial until 0xAB Char received
  //Setup RTC
// if (! RTC.begin())
// {
//   Serial.print("Couldn't find RTC");
//   while (1);
// }
//
//  if (! RTC.isrunning())
// {
//   Serial.print("RTC is NOT running!");
//   Serial.println();
// }
//   RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() 
{  
  //realtime
  DateTime now = RTC.now();
  Serial.print(now.hour());
  Serial.print(" : ");
  Serial.print(now.minute());
  Serial.print(" : ");
  Serial.print(now.second());
  Serial.println();
  
  //Read sensor
  Read_SDS011();
  Read_MQ07();
  Read_MQ136();
  Read_MQ131();
  Read_CJMCU_6814();
  //Save data vào mảng 1h (cột 1-cột 12 của mang_TB_1h); tinh trung bình 1h (cột thứ 13 của mang_TB_1h)
  Save_Data();

  //Tinh AQI_h
  Tinh_AQI_h();

  //Luu data len mang ngay
  Data_AQI_h();
  
  //Tinh AQI_d
  Tinh_AQI_d();
  
  //Luu data len mang thang
  Data_AQI_thang();

  //Truyền Data
  //Send_data();
  Wire.requestFrom(slaveAddress, sizeof x);
  wireReadData(x);
  Wire.requestFrom(slaveAddress, sizeof y);
  wireReadData(y);
  Serial.print("x: ");
  Serial.println(x);
  Serial.print("y: ");
  Serial.println(y);

  if((x<25)&&(y==35))
  {
    for(int i=0;i<7;i++)
    {
      Wire.beginTransmission(slaveAddress);
      wireWriteData(mang_AQI_h[i][x]);
      Serial.println(mang_AQI_h[i][x]);
      Wire.endTransmission();
    }
  }
  else if(x==35)
  {
    for(int i=0;i<7;i++)
    {
      Wire.beginTransmission(slaveAddress);
      wireWriteData(mang_AQI_t[i][y-1]);
      Wire.endTransmission();
    }
  }
  
}



void Read_MQ07()
{
  val_1=analogRead(MQ7);
  vRl_1=val_1/1024*5.0;
  Ratio_co=(((5.0*Rl_co)/vRl_1)-Rl_co)/R0_co;
  mq7Val =105.83* pow(Ratio_co,-1.494);  
  co = (mq7Val*1150);
}

void Read_SDS011()
{
  mySerial.readBytesUntil(0xAB,buffSDS,20);
    
  PM2_5 = ((buffSDS[3]*256)+buffSDS[2])/10;
  
  PM10 = ((buffSDS[5]*256)+buffSDS[4])/10;
}

void Read_MQ136()
{
  val_so2=analogRead(SO2);
  vRl_so2=val_so2/1024*5.0;
  Ratio_so2=(((5.0*Rl_so2)/vRl_so2)-Rl_so2)/R0_so2;
  so2Val =(40.553*pow(Ratio_so2,-1.082));
  so2 = (so2Val*2.620);
}

void Read_MQ131()
{
  val_o3=analogRead(O3);
  vRl_o3=val_o3/1024*5.0;
  Ratio_o3=(((5.0*Rl_o3)/vRl_o3)-Rl_o3)/R0_o3;
  o3Val =(8.1654*pow(Ratio_o3,2.3352));
  o3 = (o3Val*1.96);
}

void Read_CJMCU_6814()
{
  val_4=analogRead(NO2);
  vRl_4=val_4/1024*5.0;
  Ratio_4=((Rl_no2* vRl_4)/(5-vRl_4))/R0_no2;
  no2Val =(158.22*pow(Ratio_4,0.9955));
  if(no2Val<0)
  {
    no2Val=0;
  }
  no2 = (no2Val*1.88);
}

void Save_Data()
{
  DateTime now = RTC.now();
  //Lưu data vào mảng 1h (cột 1-cột 12 của mang_TB_1h)
  if ((now.minute()%5)==0 && now.second()==0)
  {
    tb_12_data(mang_TB_1h,0,PM2_5);
    tb_12_data(mang_TB_1h,1,PM10); 
    tb_12_data(mang_TB_1h,2,co);
    tb_12_data(mang_TB_1h,3,so2);
    tb_12_data(mang_TB_1h,4,o3);
    tb_12_data(mang_TB_1h,5,no2);
    //Tính trung bình 1h (cột thứ 13 của mang_TB_1h)
    for (int i=0;i<6;i++)
    {
      Tinh_Trung_binh(mang_TB_1h,i);
    }
    //In mang data trong 1h 
    Serial.println("Mang1h_12 ");
    for(int i=0;i<6;i++)
    {
      for(int j=0;j<13;j++)
      {
        Serial.print(mang_TB_1h[i][j]);
        Serial.print("  ");
        if (j==12)
          Serial.println("");
      }
    }
  }
}

void Tinh_AQI_h()
{
  DateTime now = RTC.now();
  if(now.minute()==0 && now.second()==5)
  {
    //Save data vào mang_TB_24h (cột 1-cột 24 của mang_TB_24h)
    for (int i=0;i<6;i++)
    {
      tb_24_data(mang_TB_24h,i,mang_TB_1h[i][12]);
    }
    tb_24_data(mang_TB_24h,6,mang_TB_24h[6][23]);
    TB_8h_d();
    
    //In mang Data 24h
    Serial.println("Mang24h ");
    for(int i=0;i<7;i++)
    {
      for(int j=0;j<24;j++)
      {
        Serial.print(mang_TB_24h[i][j]);
        Serial.print("  ");
        if (j==23)
          Serial.println("");
      }
    }
    
    //Tính nowcast                  
    Fine_nowcast(mang_TB_24h,0);
    nowcast2_5=nowcast;
    Serial.print("nowcast2_5: ");
    Serial.println(nowcast2_5);
    Fine_nowcast(mang_TB_24h,1);
    nowcast10=nowcast;
    Serial.print("nowcast10: ");
    Serial.println(nowcast10);

    //AQI_h
    AQI_PM2_5(nowcast2_5);
    aqi_h_pm2_5=aqi_pm2_5;
    Serial.println("PM2.5");
    Serial.print("aqi_h_pm2_5 ");
    Serial.println(aqi_h_pm2_5);
    AQI_PM10(nowcast10);
    aqi_h_pm10=aqi_pm10;
    Serial.println("PM10");
    Serial.print("aqi_h_pm10 ");
    Serial.println(aqi_h_pm10);   
    AQI_CO(mang_TB_24h[2][23]);
    aqi_h_co=aqi_co;
    Serial.println("CO");
    Serial.print("aqi_h_co ");
    Serial.println(aqi_h_co); 
    AQI_SO2(mang_TB_24h[3][23]);
    aqi_h_so2=aqi_so2;
    Serial.println("SO2");
    Serial.print("aqi_h_so2 ");
    Serial.println(aqi_h_so2);
    AQI_O3(mang_TB_24h[4][23]);
    aqi_h_o3=aqi_o3;
    Serial.println("O3");
    Serial.print("aqi_h_o3 ");
    Serial.println(aqi_h_o3); 
    AQI_NO2(mang_TB_24h[5][23]);
    aqi_h_no2=aqi_no2;
    Serial.println("NO2");
    Serial.print("aqi_h_no2 ");
    Serial.println(aqi_h_no2);   
    AQI_tonghop();
    AQI_h_max=AQI_max;
    Serial.print("AQI_h ");
    Serial.println(AQI_h_max); 

    //Hien thi led
    if(AQI_h_max < 51)
    {
      RGB_gio(0,228,0);
    }
    else if(AQI_h_max < 101)
    {
      RGB_gio(255,255,0);
    }
    else if(AQI_h_max < 151)
    {
      RGB_gio(255,126,0);
    }
    else if(AQI_h_max < 201)
    {
      RGB_gio(255,0,0);
    }
    else if(AQI_h_max < 301)
    {
      RGB_gio(143,63,151);
    }
    else
    {
      RGB_gio(126,0,35);
    }
  } 
}

void Data_AQI_h()
{
  DateTime now = RTC.now();
  //Luu data mang_AQI_d
  if(now.minute()==0 && now.second()==10)
  {
    if(now.hour()==0)
    {
      mang_AQI_h[0][23]=AQI_h_max;
      mang_AQI_h[1][23]=aqi_h_pm2_5;
      mang_AQI_h[2][23]=aqi_h_pm10;
      mang_AQI_h[3][23]=aqi_h_co;
      mang_AQI_h[4][23]=aqi_h_so2;
      mang_AQI_h[5][23]=aqi_h_o3;
      mang_AQI_h[6][23]=aqi_h_no2;
    } 
    else if(now.hour()>0 && now.hour()<24)
    {
      mang_AQI_h[0][now.hour()-1]=AQI_h_max;
      mang_AQI_h[1][now.hour()-1]=aqi_h_pm2_5;
      mang_AQI_h[2][now.hour()-1]=aqi_h_pm10;
      mang_AQI_h[3][now.hour()-1]=aqi_h_co;
      mang_AQI_h[4][now.hour()-1]=aqi_h_so2;
      mang_AQI_h[5][now.hour()-1]=aqi_h_o3;
      mang_AQI_h[6][now.hour()-1]=aqi_h_no2;
    }  
    
    //In mang Data h
    Serial.println("Mang AQI h");
    for(int i=0;i<7;i++)
    {
      for(int j=0;j<24;j++)
      {
        Serial.print(mang_AQI_h[i][j]);
        Serial.print("  ");
        if (j==23)
          Serial.println("");
      }
    }  
  }
}

void Tinh_AQI_d()
{
  DateTime now = RTC.now();
  if((now.hour()==0)&&(now.minute()==0)&&(now.second()==15))
  {
    Tinh_Trung_binh_d(mang_TB_24h,0);
    Tinh_Trung_binh_d(mang_TB_24h,1);
    TB_1h_max(mang_TB_24h,2);
    TB_1h_max(mang_TB_24h,3);
    TB_1h_max(mang_TB_24h,4);
    TB_1h_max(mang_TB_24h,5);
    TB_1h_max(mang_TB_24h,6);
    //In mang Data 24h
    Serial.println("Mang24h_cot25 ");
    for(int i=0;i<7;i++)
    {
      for(int j=0;j<25;j++)
      {
        Serial.print(mang_TB_24h[i][j]);
        Serial.print("  ");
        if (j==24)
          Serial.println("");
      }
    }
    if(mang_TB_24h[6][24]>400)
    {
      tb_maxo3=mang_TB_24h[4][24];
    }
    else
    {
      tb_maxo3=max(mang_TB_24h[4][24],mang_TB_24h[6][24]);
    }
    
    //AQI_d
    AQI_PM2_5(mang_TB_24h[0][24]);
    aqi_d_pm2_5=aqi_pm2_5;
    Serial.println("PM2.5");
    Serial.print("aqi_d_pm2_5 ");
    Serial.println(aqi_d_pm2_5);
    AQI_PM10(mang_TB_24h[1][24]);
    aqi_d_pm10=aqi_pm10;
    Serial.println("PM10");
    Serial.print("aqi_d_pm10 ");
    Serial.println(aqi_d_pm10);  
    AQI_CO(mang_TB_24h[2][23]);
    aqi_d_co=aqi_co;
    Serial.println("CO");
    Serial.print("aqi_d_co ");
    Serial.println(aqi_d_co); 
    AQI_SO2(mang_TB_24h[3][24]);
    aqi_d_so2=aqi_so2;
    Serial.println("SO2");
    Serial.print("aqi_d_so2 ");
    Serial.println(aqi_d_so2);
    AQI_O3(tb_maxo3);
    aqi_d_o3=aqi_o3;
    Serial.println("O3");
    Serial.print("aqi_d_o3 ");
    Serial.println(aqi_d_o3); 
    AQI_NO2(mang_TB_24h[5][24]);
    aqi_d_no2=aqi_no2;
    Serial.println("NO2");
    Serial.print("aqi_d_no2 ");
    Serial.println(aqi_d_no2);   
    AQI_tonghop();
    AQI_d_max=AQI_max;
    Serial.print("AQI_d ");
    Serial.println(AQI_d_max); 

        //Hien thi led
    if(AQI_d_max < 51)
    {
      RGB_ngay(0,228,0);
    }
    else if(AQI_d_max < 101)
    {
      RGB_ngay(255,255,0);
    }
    else if(AQI_d_max < 151)
    {
      RGB_ngay(255,126,0);
    }
    else if(AQI_d_max < 201)
    {
      RGB_ngay(255,0,0);
    }
    else if(AQI_d_max < 301)
    {
      RGB_ngay(143,63,151);
    }
    else
    {
     RGB_ngay(126,0,35);
    }
  }
}

void Data_AQI_thang()
{
  DateTime now = RTC.now();
  //Luu data mang_AQI_t
  if((now.hour()==0)&&(now.minute()==0)&&(now.second()==20))
  {
    mang_AQI_t[0][now.day()-1]=AQI_d_max;
    mang_AQI_t[1][now.day()-1]=aqi_d_pm2_5;
    mang_AQI_t[2][now.day()-1]=aqi_d_pm10;
    mang_AQI_t[3][now.day()-1]=aqi_d_co;
    mang_AQI_t[4][now.day()-1]=aqi_d_so2;
    mang_AQI_t[5][now.day()-1]=aqi_d_o3;
    mang_AQI_t[6][now.day()-1]=aqi_d_no2;

    //In mang Data 30d
    Serial.println("Mang AQI thang");
    for(int i=0;i<7;i++)
    {
      for(int j=0;j<31;j++)
      {
        Serial.print(mang_AQI_t[i][j]);
        Serial.print("  ");
        if (j==30)
          Serial.println("");
      }
    }  
  }
}

void Fine_nowcast(float a[7][25],int i)
{
  float ww,w,cmin,cmax;
  nowcast=0;
  cmin=a[i][23];
  cmax=a[i][23];
  for(int j=22;j>11;j--)
  {
      if(cmin>a[i][j])
          cmin=a[i][j];
      if(cmax<a[i][j])
          cmax=a[i][j];
  }
  ww=cmin/cmax;
  if(ww<=0.5)
  {
    float w=0.5;
    for(int u=0;u<12;u++)
    {
      mang_nowcast[u]=pow(0.5,u+1)*a[i][23-u];
    }
    for(int k=0;k<12;k++)
    {
      nowcast = nowcast + mang_nowcast[k];
    }
  }
  else
  {
    float w=ww;
    float tong_1, tong_2 =0;
    for(int u=0;u<12;u++)
    {
      mang_nowcast[u]=pow(w,u)*a[i][23-u];
    }
    for(int h=0;h<12;h++)
    {
      tong_1 = tong_1 + mang_nowcast[h];
      tong_2 = tong_2 + (pow(w,h));
    }
    nowcast = tong_1 / tong_2;
  }
}

void Tinh_Trung_binh(float a[6][13],int i)
{
  float tong=0;
  for(int j=0;j<12;j++)
  {
    tong = tong + a[i][j];
  }
  a[i][12] = tong / 12;
}

void Tinh_Trung_binh_d(float a[7][25],int i)
{
  float tong=0;
  for(int j=0;j<24;j++)
  {
    tong = tong + a[i][j];
  }
  a[i][24] = tong / 24;
}

void AQI_PM2_5(float x)
{
  if(x<25)
    aqi_pm2_5 = 2* x;
  else if(x<50)
    aqi_pm2_5 = 2*(x-25)+ 50;
  else if(x<80)
    aqi_pm2_5 = x +50;
  else if(x<150)
    aqi_pm2_5 = (0.7143)*(x-80)+150;
  else if(x<350)
    aqi_pm2_5 = x +50;
  else
    aqi_pm2_5 = (0.6667)*(x-350)+400; 
}

void AQI_PM10(float x)
{
  if(x<50)
    aqi_pm10 = x;
  else if(x<150)
    aqi_pm10 = (0.5)*(x-50)  + 50;
  else if(x<250)
    aqi_pm10 = (0.5)*(x-150) +100;
  else if(x<350)
    aqi_pm10 = (0.5)*(x-250) +150;
  else if(x<420)
    aqi_pm10 = (1.4286)*(x-350)+200;
  else if(x<500)
    aqi_pm10 = (1.25)*(x-420) +300;
  else
    aqi_pm10 = x-100; 
}

void AQI_CO(float x)
{
  if(x<10000)
    aqi_co = 0.005*x;//(1/200)*x
  else if(x<30000)
    aqi_co = (0.0025)*(x-10000)  + 50;
  else if(x<45000)
    aqi_co = (0.0033)*(x-30000) + 100;
  else if(x<60000)
    aqi_co = (0.0033)*(x-45000) + 150;
  else if(x<90000)
    aqi_co = (0.0033)*(x-60000) + 200;
  else if(x<120000)
    aqi_co = (0.0033)*(x-90000) + 300;
  else
    aqi_co = (0.0033)*(x-120000)+ 400; 
}

void AQI_SO2(float x)
{
  if(x<125)
    aqi_so2 = (0.4)*x;
  else if(x<350)
    aqi_so2 = (0.2222)*(x-125)  + 50;
  else if(x<550)
    aqi_so2 = (0.25)*(x-350) + 100;
  else if(x<800)
    aqi_so2 = (0.2)*(x-550) + 150;
  else if(x<1600)
    aqi_so2 = (0.125)*(x-800) + 200;
  else if(x<2100)
    aqi_so2 = (0.2)*(x-1600) + 300;
  else
    aqi_so2 = (0.1887)*(x-2100)+ 400; 
}

void AQI_O3(float x)
{
  if(x<160)
    aqi_o3 = (0.3125)*x;
  else if(x<200)
    aqi_o3 = (1.25)*(x-160)  + 50;
  else if(x<300)
    aqi_o3 = (0.5)*(x-200) + 100;
  else if(x<400)
    aqi_o3 = (0.5)*(x-300) + 150;
  else if(x<800)
    aqi_o3 = (0.25)*(x-400) + 200;
  else if(x<1000)
    aqi_o3 = (0.5)*(x-800) + 300;
  else
    aqi_o3 = (0.5)*(x-1000)+ 400; 
}

void AQI_NO2(float x)
{
  if(x<100)
    aqi_no2 = (0.5)*x;
  else if(x<200)
    aqi_no2 = (0.5)*(x-100)  + 50;
  else if(x<700)
    aqi_no2 = (0.1)*(x-200) + 100;
  else if(x<1200)
    aqi_no2 = (0.1)*(x-700) + 150;
  else if(x<2350)
    aqi_no2 = (0.0869)*(x-1200) + 200;
  else if(x<3100)
    aqi_no2 = (0.1333)*(x-2350) + 300;
  else
    aqi_no2 = (0.1333)*(x-3100)+ 400; 
}

void AQI_tonghop()
{
  float mang_AQI[6]={aqi_pm2_5,aqi_pm10,aqi_co,aqi_so2,aqi_o3,aqi_no2};
  AQI_max=mang_AQI[0];
  for(int i=1;i<6;i++)
  {
      if(AQI_max<mang_AQI[i])
          AQI_max=mang_AQI[i];
  }
}

void tb_12_data(float a[6][13],int i,float data)
{
  for(int j=0;j<11;j++)
  {
    a[i][j]=a[i][j+1];
  }
  a[i][11]=data;
}

void tb_24_data(float a[7][25],int i,float data)
{
  for(int j=0;j<23;j++)
  {
    a[i][j]=a[i][j+1];
  }
  a[i][23]=data;
}

void TB_1h_max(float a[7][25],int i)
{
  a[i][24]=a[i][0];
  for(int u=0;u<24;u++)
  {
      if(a[i][24]<a[i][1u])
          a[i][24]=a[i][u];
  }
}

void TB_8h_d()
{
  float tong=0;
  for(int i=16;i<24;i++)
  {
    tong = tong + mang_TB_24h[4][i];
  }
  mang_TB_24h[6][23] = tong / 8;
}

void RGB_gio ( int red_light_value, int green_light_value, int blue_light_value )
 {
  analogWrite ( 7, red_light_value ) ;
  analogWrite ( 6, green_light_value ) ;
  analogWrite ( 5, blue_light_value ) ;
}
void RGB_ngay ( int red_light_value, int green_light_value, int blue_light_value )
 {
  analogWrite ( 4, red_light_value ) ;
  analogWrite ( 3, green_light_value ) ;
  analogWrite ( 2, blue_light_value ) ;
}
