/***********************************************************************/
/* FILE : KOLOM.HPP                                                    */
/* KELAS UNTUK MENGHITUNG HARGA DAN KENDALA PADA KOLOM                 */
/* DITULIS OLEH YOHAN NAFTALI (JULY 1999)                              */
/***********************************************************************/
class kolom
{
   private:

   /****************************************************/
   /* PENDEKLARASIAN VARIABEL PRIVATE PADA KELAS KOLOM */
   /****************************************************/

   ///////////////////////
   // Perhitungan Kolom //
   ///////////////////////
   float RHO; 	  // Rasio penulangan
   float PN;	  // Gaya aksial nominal
   float MNX;    // Momen nominal arah x
   float MNY;	  // Momen nominal arah y
   float MOX;    // Momen ekivalen arah x
   float MOY;	  // Momen ekivalen arah y
   float PO;	  // Batas gaya aksial karena eksentrisitas minimum (N)
   float PNcoba; // Gaya aksial nominal coba
   float PNB;	  // Gaya aksial nominal pada kondisi balance
   float MNcoba; // Momen nominal coba
   float beta;	  // Faktor bentuk biaksial
   float ASTOT;  // Luas besi total pada penampang (mm^2)
   float CB;  	  // Jarak serat tekan terluar ke garis netral
   				  // pada keadaan	balance (mm)
   float FS;     // Tegangan dalam tulangan pada beban kerja
   float eks;	  // Eksentrisitas (mm)
   float ekscoba;// Eksentirsitas maksimum (mm)
   float eksb;   // Eksentrisitas pada keadaan balance (mm)
   float epsb;	  // Regangan baja pada keadaan balance (mm)
   float epsy;	  // Regangan baja pada keadaan luluh (mm)

   float rasio_kelangsingan; // Rasio kelangsingan

   float fs1b; // Tegangan baja tulangan pada keadaan balance (MPa)
   float fsi;  // Tegangan baja (MPa)
   float di;   // Jarak tulangan ke garis netral (mm)
   float Fsi;  // Gaya Yang Disumbangkan Oleh Tulangan (N)
   float ASdi; // Luas tulangan pada jarak di
   float fsf1;
   float d11;

   /////////////////////////////////////////////////////////////
   // Variabel Pembantu Untuk Menyelesaikan Persamaan Kuardat //
   /////////////////////////////////////////////////////////////
   float asol;
   float bsol;
   float csol;
   float dsol;

   ///////////////////////////////////////
   // Variabel Pada Metoda False Posisi //
   ///////////////////////////////////////
   int iterasi_fp; 		// Jumlah iterasi dengan metode false posisi

   float trialkiri;  // Titik coba pertama
   float trialkanan; // Titik coba kedua
   float trialbaru;  // Titik coba baru
   float deki;			// Nilai titik coba pertama
   float deka;			// Nilai titik coba kedua
   float deba;			// Nilai titik coba baru

   /*************************************/
   /* PROTOTYPE FUNGSI PADA KELAS KOLOM */
   /*************************************/
   void rho();
   void jarak_tulangan();
   void kelangsingan();
   void analisa();
   void sengkang_kolom();
   float hitung_kolom (float Ccoba);

   public:

   /***************************/
   /* CONSTRUCTOR CLASS KOLOM */
   /***************************/
	kolom()
	{
      /////////////////////////////
      // Inisialisasi Nilai Awal //
      /////////////////////////////
      kendala=0.;
      harga=0.;

      /////////////////////////////////////
      // Memangsil Fungsi-Fungsi Kendala //
      /////////////////////////////////////
     	rho();
   	jarak_tulangan();
      kelangsingan();
      analisa();
      sengkang_kolom();

      //////////////////////////////
      // Menghitung Kendala Total //
      //////////////////////////////
      kendala=kendala_sengkang
      		 +kendala_r
             +kendala_tul
             +kendala_gaya
             +kendala_kelangsingan;

      //////////////////////////////////
      // Perhitungan Berat Dan Volume //
      //////////////////////////////////
      volume_beton=sisi*sisi*L/1.E6;
   	berat_besi=ASTOT*L*bj_besi/1.E6;
      berat_sengkang=(fabs(L/(Jarak_S/1000.))-1.)
                    *4.*((sisi-(2.*selimut_kolom))/1000.)
                    *(pi/4.)*pow((DIAS/1000.),2.)
                    *bj_besi;

      //////////////////////
      // Menghitung Harga //
      //////////////////////
      harga=(volume_beton*harga_beton)
           +(berat_besi*harga_besi)
           +(berat_sengkang*harga_besi);
	}
};

/****************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA RASIO PENULANGAN */
/****************************************************/
void kolom::rho()
{
   ASTOT=((4.*N_DIA-4.)*(pi/4.)*(pow(DIA,2)));
   RHO=(ASTOT/(pow(sisi,2)));

   ////////////////////////////////////
   // Menghitung Kendala Rho Minimum //
   ////////////////////////////////////
   kendala_r_min=((0.01/RHO)-1.);
   if(kendala_r_min<0.)
   {
   	kendala_r_min=0.;
   }

   /////////////////////////////////////
   // Menghitung Kendala Rho Maksimum //
   /////////////////////////////////////
   kendala_r_mak=((RHO/0.08)-1.);
   if(kendala_r_mak<0.)
   {
   	kendala_r_mak=0.;
   }

   //////////////////////////////
   // Menjumlahkan Kendala Rho //
   //////////////////////////////
   kendala_r=kendala_r_min+kendala_r_mak;
}

/*********************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA JARAK ANTARA TULANGAN */
/*********************************************************/
void kolom::jarak_tulangan()
{
   ////////////////////////////////////////////
	// Mengecek Jarak Minimum Antara Tulangan //
   ////////////////////////////////////////////
   float min1=(1.5*DIA);
   float min2=40.;
   float sisa=sisi-(2.*selimut_kolom)-N_DIA*DIA;

   jarak_antar_tulangan=sisa/(N_DIA-1.);
   if(jarak_antar_tulangan==0)
   {
      jarak_antar_tulangan=limit_nol;
   }

   float jarak_min=min1;
   if(min2<min1)
   {
   	jarak_min=min2;
   }

   //////////////////////////////////////////////////////////////////
   // Menghitung Kendala Total Akibat Kendala Jarak Antar Tulangan //
   //////////////////////////////////////////////////////////////////
   kendala_tul=((jarak_min/jarak_antar_tulangan)-1.);
   if(kendala_tul<0.)
   {
   	kendala_tul=0.;
   }
}

/******************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA KELANGSINGAN KOLOM */
/******************************************************/
void kolom::kelangsingan()
{
   ///////////////////////////////////////////////////////////
   // Syarat Kelangsingan Kolom Menurut SK SNI T-15-1991-03 //
   // KL/r <= 22                                            //
   ///////////////////////////////////////////////////////////
	_K=0.5;
   rasio_kelangsingan=(_K*L*1000./(sisi*(sqrt((1./12.)))));

   /////////////////////////////////////
   // Menghitung Kendala Kelangsingan //
   /////////////////////////////////////
   kendala_kelangsingan=(rasio_kelangsingan/22.)-1.;
   if(kendala_kelangsingan<0)
   {
   	kendala_kelangsingan = 0.;
   }
}

/*************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA GAYA STRUKTUR */
/*************************************************/
void kolom::analisa()
{
   ///////////////////////////////////
   // Persiapan Gaya Yang Membebani //
   ///////////////////////////////////
   PN=fabs(PU/teta); // (N)
   MNX=fabs(MUX/teta)*1000.; // (Nm) -> (Nmm)
   MNY=fabs(MUY/teta)*1000.; // (Nm) -> (Nmm)

   //////////////////////////////////////////////////
   // Menginisialisasi Gaya Aksial Bila Bernilai 0 //
   //////////////////////////////////////////////////
   if(PN==0.)
   {
   	PN=limit_nol; // (N)
   }

   //////////////////////////////////////////
   // Periksa Batas Gaya Aksial            //
   // Sebagai Syarat Eksentrisitas Minimum //
   //////////////////////////////////////////
   PO=teta*(0.85*FC*(pow(sisi,2))+(ASTOT*FY)); // (N)

   ///////////////////////////
   // Menghitung Kendala PO //
   ///////////////////////////
   kendala_po=((PN/PO)-1.);
   if(kendala_po<=0.)
   {
   	kendala_po=0.;

   	//////////////////////////////////////////////
	   // Perhitungan Jarak Pusat Tulangan Ke Tepi //
	   //////////////////////////////////////////////
	   DS=selimut_kolom+(0.5*DIA); // (mm)
	   D=sisi-DS; // (mm)

   	//////////////////////////////////////////////////////////////
	   // Menghitung Faktor Pengali Tinggi Blok Tegangan Segiempat //
	   // Ekivalen (B1)                                            //
	   // Sumber : SK SNI T-15-1991-03 Pasal 3.3.2 butir 7.(3)     //
	   //////////////////////////////////////////////////////////////
   	if(FC<=30.)
   	{
   		BT1=0.85;
	   }
   	else
	   {
   		BT1=0.85-0.008*(FC-30.);
	   }
	   if(BT1<0.65)
	   {
	   	BT1=0.65;
   	}

	   ////////////////////////////////////////////////////////////
	   // Perhitungan Faktor Bentuk Kolom Biaksial               //
	   // Untuk Symetrical Arrangement Of Reinforcement          //
	   // Rumus Empiris Dari Hulse dan Mosley (1986)             //
	   // Buku : Reinforced Concrete Design by Computer hal. 163 //
	   ////////////////////////////////////////////////////////////
	   beta=0.3+(0.7/0.6)*(0.6-(PU/(sisi*sisi*FC)));

	   /////////////////////////////////////
	   // Dengan Nilai beta Minimum = 0.3 //
	   /////////////////////////////////////
	   if(beta<0.3)
	   {
	   	beta=0.3;
	   }

	   /////////////////////////////////////
	   // Mencari Momen Ekivalen Biaksial //
	   /////////////////////////////////////
	   if(MNX > MNY)
	   {
	      MOX=MNX+(MNY*((1.-beta)/beta)); // Momen ekivalen (Nmm)
	      eks=(fabs(MOX/PN)); // (mm)

	   }
	   else
	   {
	      MOY=MNY+(MNX*((1.-beta)/beta)); // Momen ekivalen (Nmm)
	      eks=(fabs(MOY/PN)); // (mm)
	   }

   	////////////////////////////////////////////////////////
	   // Menghitung Letak Garis Netral Pada Keadaan Balance //
   	////////////////////////////////////////////////////////
	   CB=600.*D/(FY+600.); // (mm)

   	//////////////////////////////////////////////
	   // Menghitung Tegangan Pada Keadaan Balance //
   	//////////////////////////////////////////////
	   deki=hitung_kolom(CB);
   	eksb=ekscoba; // (mm)
	   epsb=0.003*(CB-DS)/CB;
   	epsy=FY/2.E5;

	   if(epsb>epsy)
   	{
   		fs1b=FY; // (Mpa)
	   }
   	if(epsb<epsy)
	   {
   		fs1b=epsb*2.E5;	// (MPa)
	   }

   	//--------------------------------------------------------------//
   	// Proses Mencari Letak Garis Netral Dengan Metode False Posisi //
	   //--------------------------------------------------------------//

	   ////////////////////////////////
	   // Pembagian Daerah Pencarian //
	   ////////////////////////////////
	   trialkiri=40.;
	   trialkanan=(sisi-40.);
	   trialbaru=(CB);

	   ///////////////////////////
   	// Menghitung Nilai Awal //
	   ///////////////////////////
	   deki=hitung_kolom(trialkiri);
	   deka=hitung_kolom(trialkanan);
	   deba=hitung_kolom(trialbaru);

	   /////////////////////////
	   // Periksa Batas Range //
	   /////////////////////////
	   if((deki*deka>0)&&(deki!=0)&&(deka!=0))
	   {
	   	do
	   	{
    			/////////////////////////////////////////////////////
   	   	// Bila Nilai Yang Kita Cari Tidak Ada Dalam Range //
	    		/////////////////////////////////////////////////////
   	      if(trialkiri>(limit_nol))
      	   {
	      		trialkiri=((trialkiri)/2.);
   	         deki=hitung_kolom(trialkiri);
      	   }
      		else
	         {
   	        	trialkanan+=10.;
      	      deka=hitung_kolom(trialkanan);
         	   if(trialkanan>(sisi/0.85))
            	{
	            	break;
   	         }
      	   }
	   	}while(deki*deka>0&&(deki!=0)&&(deka!=0));
   	}

	   //////////////////////////////
   	// Akselerator False Posisi //
	   //////////////////////////////
   	iterasi_fp=1;
	   do
   	{
	      /////////////////////////
   	   // Periksa Konvergensi //
      	// Diambil E = 1 mm    //
	      /////////////////////////
   	   if(deba<1.&&deba>(-1.))
      	{
         	if(PNcoba<=0)
	         {
   	      	PNcoba=limit_nol;
      	   }
         	MNcoba=fabs(MNcoba);
	         break;
   	   }

	      /////////////////////////////////////////////
   	   // Periksa Apakah deki Dan deba Sama Tanda //
      	/////////////////////////////////////////////
	   	if((deki*deba)<0.)
   		{
      	   ///////////////////////////////////////
         	// Geser trialkanan menuju trialbaru //
	         ///////////////////////////////////////
   			trialkanan=trialbaru;
      	   deka=hitung_kolom(trialkanan);
		   }
   		else
	   	{
	         //////////////////////////////////////
   	      // Geser trialkiri menuju trialbaru //
      	   //////////////////////////////////////
  		 		trialkiri=trialbaru;
	        	deki=hitung_kolom(trialkiri);
		  	}

	   	/////////////////////////////////////
   		// Persempit Lagi Daerah Pencarian //
   		/////////////////////////////////////
	   	trialbaru=((trialkiri+trialkanan)/2.);

   		deba=hitung_kolom(trialbaru);

	      iterasi_fp++;

         if(trialkiri==trialkanan)
         {
         	break;
         }

	   }while(iterasi_fp<=100);

   	////////////////////////////////
	   // Insialisasi Jumlah Iterasi //
   	////////////////////////////////
	   iterasi_fp=1;

	   /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
   	/* Proses Pencarian Dengan Metoda False Posisi Dimulai */
	   /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
   	do
	   {
         if(trialkiri==trialkanan)
         {
         	break;
         }
   		////////////////////////////////
      	// Menentukan Titik Coba Baru //
	      ////////////////////////////////
   	   trialbaru=trialkanan-((deka)*(trialkanan-trialkiri)
      				         	/((deka)-(deki)));

   		deba=hitung_kolom(trialbaru);

	      /////////////////////////
   	   // Periksa Konvergensi //
   	   // Diambil E = 10 mm   //
	      /////////////////////////
	      if(deba<10.&&deba>(-10.))
   	   {
	         if(PNcoba<=0)
   	      {
      	   	PNcoba=limit_nol;
         	}
	         MNcoba=fabs(MNcoba);
   	      break;
      	}

	      /////////////////////////////////////////////
   	   // Periksa Apakah deki Dan deba Sama Tanda //
      	/////////////////////////////////////////////
	      if((deki*deba)<0.)
   	   {
	         ///////////////////////////////////////
   	      // Geser trialkanan menuju trialbaru //
      	   ///////////////////////////////////////
         	trialkanan=trialbaru;
	         deka=hitung_kolom(trialkanan);

   	  	}
      	else
	      {
   	      //////////////////////////////////////
      	   // Geser trialkiri menuju trialbaru //
         	//////////////////////////////////////
	   	 	trialkiri=trialbaru;
   	      deki=hitung_kolom(trialkiri);

	   	}

   	   iterasi_fp++;
      	if(iterasi_fp>=100)
	      {
   	      ///////////////////////////////////////
      	   // Dihitung Dengan Metoda Pendekatan //
         	///////////////////////////////////////
	         deka=hitung_kolom((trialkiri+trialkanan)/2.);
   	   	break;
      	}
	   }while(deki*deka!=0.);
   	FPU=fabs(PNcoba);				// Dalam N
	   FMU=fabs(MNcoba)/1000.;		// Hasil perhitungan dalam Nmm -> Nm

	   ///////////////////////////////////////////
   	// Menghitung Kendala Akibat Gaya Aksial //
	   ///////////////////////////////////////////
  		kendala_pn=((PN/PNcoba)-1.);
	   if(kendala_pn<0)
   	{
	   	kendala_pn=0.;
   	}

	   ///////////////////////////////////////////
   	// Menghitung Kendala Akibat Gaya Lentur //
	   ///////////////////////////////////////////
  		if(MNX > MNY)
	  	{
   	   kendala_mn=((MOX/MNcoba)-1.);
      	if(kendala_mn<0)
	      {
   	   	kendala_mn=0.;
      	}
	  	}
  		else
	  	{
   	   kendala_mn=((MOY/MNcoba)-1.);
      	if(kendala_mn<0)
	      {
   	   	kendala_mn=0.;
      	}
	  	}
   }

   //////////////////////////////////////////////////////////
   // Bila Gaya Aksial Melanggar Gaya Aksial Batas Minimum //
   // Eksentrisitas Pada Kolom Berpengikat Sengkang        //
   //////////////////////////////////////////////////////////
   else
   {
      FPU=PO;
      FMU=0.;
      kendala_pn=0.;
      kendala_mn=0.;
   }

  	///////////////////////////////////////////////////////
   // Menghitung Kendala Total Akibat Gaya Yang Terjadi //
   ///////////////////////////////////////////////////////
   kendala_gaya=kendala_po+kendala_pn+kendala_mn;
}

/***********************************************/
/* FUNGSI UNTUK MENGHITUNG TEGANGAN PADA KOLOM */
/***********************************************/
float kolom::hitung_kolom(float Ccoba)
{
   ////////////////////////////////////////
   // Menghitung Gaya Yang Ditahan Beton //
   ////////////////////////////////////////
   PNcoba=(0.85*BT1*FC*Ccoba*sisi);
   MNcoba=(0.85*BT1*FC*Ccoba*sisi)*((sisi/2.)-(BT1*Ccoba/2.));

   ///////////////////////////////////////////////////
   // Iterasi Untuk Kekuatan Yang Didukung Tulangan //
   ///////////////////////////////////////////////////
   for(int ikl=1;ikl<=N_DIA;ikl++)
   {
      di=(DS+((N_DIA-ikl)*(sisi-(2.*DS))/(N_DIA-1.)));

      if(di==(0.5*sisi))
      {
      	continue;
      }

      ///////////////////////////////////
      // Luas Tulangan Pada Baris Luar //
      ///////////////////////////////////
   	if((ikl==1)||(ikl==N_DIA))
      {
      	ASdi=(N_DIA*(pi/4.)*pow(DIA,2));
      }

      ////////////////////////////////////
      // Luas Tulangan Pada Baris Dalam //
      ////////////////////////////////////
      else
      {
      	ASdi=(2.*(pi/4.)*pow(DIA,2));
      }

      /////////////////////////////////////////////
      // Antisipasi Nilai Ccoba Dengan Limit Nol //
      /////////////////////////////////////////////
      if(Ccoba==0)
      {
      	Ccoba=limit_nol;
      }

      //////////////////////////////////////////
      // Hitung Tegangan Tahanan Tulangan     //
      // fsi = Regangan * Modulus Elastisitas //
      // Dengan Regangan = 0.003[(c-di)/c]    //
      // Modulus Elastisitas = 2E5 MPa (Baja) //
      //////////////////////////////////////////
      fsi=((600.*(Ccoba-di))/Ccoba);

      ////////////////////////////
      // Periksa Range Tegangan //
      ////////////////////////////
      if(fsi>FY)
      {
      	fsi=FY;
      }
      else if(fsi<(-FY))
      {
      	fsi=(0.-FY);
      }

      /////////////////////////////////////////////
      // Reduksi Tegangan Baja Pada Daerah Desak //
      /////////////////////////////////////////////
      if(di<(BT1*Ccoba))
      {
      	fsi=(fsi-(0.85*FC));
      }

      /////////////////////////////////////////////////
      // Hitung Gaya Yang Disumbangkan Oleh Tulangan //
      /////////////////////////////////////////////////
      Fsi=fsi*ASdi;

      ///////////////////////////////////////////////////
      // Hitung Gaya Aksial Dan Momen Tahanan Maksimal //
      ///////////////////////////////////////////////////
      PNcoba=(PNcoba+Fsi);
      MNcoba=(MNcoba+(Fsi*((sisi/2.)-di)));
   }

   /////////////////////////////////////
   // Antisipasi Divide By Zero Error //
   /////////////////////////////////////
   if (PNcoba<=0)
   {
    	PNcoba=limit_nol;
   }

   ////////////////////////////////////////
   // Hitung Momen Dan Gaya Aksial Total //
   ////////////////////////////////////////
   ekscoba=(MNcoba/PNcoba);

   /////////////////////////////
   // Kembalikan Nilai Fungsi //
   /////////////////////////////
   return (eks-ekscoba);
}

/*********************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA GAYA GESER PADA KOLOM */
/*********************************************************/
void kolom::sengkang_kolom()
{
   //////////////////////////////////////////////////////////
   // Menghitung Antar Jarak Sengkang Maksimal             //
   // Berdasarkan SK SNI T-15-1991-03 pasal 3.16.10 ayat 5 //
   //////////////////////////////////////////////////////////
   float Smak[3];
  	Smak[0]=sisi;
   Smak[1]=16.*DIA;
   Smak[2]=48.*DIAS;
   float SmakS=Smak[0];

   ////////////////////////
   // Cari Yang Terkecil //
   ////////////////////////
   for (int sk=1;sk<3;sk++)
   {
   	if(SmakS>Smak[sk])
      {
      	SmakS=Smak[sk];
      }
   }
   
   Sref=SmakS;
   /////////////////////////////////////////////
   // Menghitung Kendala Jarak Antar Sengkang //
   /////////////////////////////////////////////
   kendala_sengkang=((Jarak_S/SmakS)-1.);
   if(kendala_sengkang<0.)
   {
   	kendala_sengkang=0.;
   }
}

//---------------------------------------------------------------------//
