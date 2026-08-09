/***********************************************************************/
/* FILE : BALOK.HPP                                                    */
/* KELAS UNTUK MENGHITUNG HARGA DAN KENDALA PADA BALOK                 */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/
class balok
{
	private:

   /****************************************************/
   /* PENDEKLARASIAN VARIABEL PRIVATE PADA KELAS BALOK */
   /****************************************************/

   ///////////////////////
   // Perhitungan Balok //
   ///////////////////////
   float FS;     // Tegangan pada tulangan tarik (N/mm^2)
   float FS1;    // Tegangan pada tulangan desak (N/mm^2)
   float ARM;    // Lengan pusat tekan ke garis Netral (mm)
   float ARMS;   // Lengan pusat tulangan ke garis Netral (mm)
   float EPS;    // Regangan pada tulangan tarik (mm)
   float EPS1;   // Regangan pada tulangan tekan (mm)
   float EPSY;   // Regangan ijin baja pada keadaan luluh (mm)
   float RHB;    // Rasio tulangan balance
   float RHO;    // Rasio tulangan tarik
   float RH1;    // Rasio tulangan desak
   float RMIN;   // Rasio tulangan minimum
   float SmakS;  // Jarak antar sengkang maksimum yang diijinkan (mm)

   /////////////////////////////////////////
   // Variabel Pembantu Persamaan Kuardat //
   /////////////////////////////////////////
   float ASOL;
	float BSOL;
   float CSOL;
   float DSOL;

   /*************************************/
   /* PROTOTYPE FUNGSI PADA KELAS BALOK */
   /*************************************/
   void analisa();
   void sengkang_balok();
   public:

   /***************************/
   /* CONSTRUCTOR CLASS BALOK */
   /***************************/
   balok()
   {
      ///////////////////////////////////////
      // Inisialisai Kendala Dan Harga = 0 //
      ///////////////////////////////////////
      kendala=0.;
      harga=0.;

      //////////////////////////////////////////////
      // Menghitung Luas Tulangan Tarik Dan Tekan //
      //////////////////////////////////////////////
      AS=(0.25*pi*(pow(DIA1,2))*NL1);
      AS1=(0.25*pi*(pow(DIA2,2))*NL2);

      /////////////////////////////////////
      // Memanggil Fungsi Fungsi Kendala //
      /////////////////////////////////////
		analisa();
      sengkang_balok();
      kendala=kendala_sb+kendala_rho+kendala_M;

      //////////////////////////////////
      // Perhitungan Berat Dan Volume //
      //////////////////////////////////
      volume_beton=B*H*0.5*L/1.E6;
   	berat_besi=(AS+AS1)*0.5*L*bj_besi/1.E6;
      berat_sengkang=0.5*(fabs(L/(Jarak_S/1000.))-1.)*
      	            (
                     	2.*
                        (
                        	( (B-(2.*selimut_balok)) /1000.)+
                           2.*((H-(2.*selimut_balok))/1000.)
                        )
                     )*
                     (pi/4.)*pow((DIAS/1000.),2)*bj_besi;

      //////////////////////
      // Menghitung Harga //
      //////////////////////
      harga=volume_beton*harga_beton
           +berat_besi*harga_besi
           +berat_sengkang*harga_besi;
   }
};

/*****************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA TEGANGAN MATERIAL */
/*****************************************************/
void balok::analisa()
{
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

   DS=selimut_balok+(0.5*DIA1);
   
   D=H-DS;
   FS=FY;

   EPSY=FY/200000.;

   ///////////////////
   //Asumsi Pertama //
   ///////////////////
   FS1=FS;

   /////////////////////////////////////////////////
   // Hitung Besarnya Lengan Ke Garis Netral (mm) //
   /////////////////////////////////////////////////
   ARM=((AS-AS1)*FY)/(0.85*FC*B);
   if(ARM==0)
   {
   	ARM=limit_nol;
   }

   ////////////////////////////////////
   // Periksa Regangan Pada Tulangan //
   ////////////////////////////////////
   EPS1=0.003*(ARM-BT1*DS)/ARM;
   EPS=0.003*(BT1*D-ARM)/ARM;

   //////////////////////////////////////////////////////////////////
   // Hitung Ulang Regangan Pada Baja Apabila Tulangan Tidak Luluh //
   //////////////////////////////////////////////////////////////////
   if((EPS<EPSY)||(EPS1<EPSY))
   {
      /////////////////////////////////////////////////////////////
      // Hitung Koefisien Persamaan Kuardat Dan Menyelesaikannya //
      /////////////////////////////////////////////////////////////
      ASOL=0.85*FC*B;
      BSOL=600.*AS1-AS*FY;
      CSOL=(-600.)*BT1*AS1*DS;
      DSOL=pow(BSOL,2)-(4.*(ASOL*CSOL));
      if(DSOL<=0)
      {
      	DSOL=0.;
      }
      ARMS=(-BSOL)+sqrt(DSOL);
      if(ARMS<=0&&DSOL>=0)
      {
      	ARMS=(-BSOL)-sqrt(DSOL);
      }
      ARM=0.5*(ARMS/ASOL);
      if(ARM<=0)
      {
      	ARM=limit_nol;
      }

      ///////////////////////////////////
      // Hitung Regangan Pada Tulangan //
      ///////////////////////////////////
      EPS1=0.003*(ARM-BT1*DS)/ARM;
      EPS=0.003*(BT1*D-ARM)/ARM;
   }

   if(EPS>EPSY)
   {
   	FS=FY;
   }

   FS1=EPS1*200000.;
   if(FS1>FY)
   {
   	FS1=FY;
   }
   if(FS1<0)
   {
   	FS1=0;
   }

   //////////////////////////////////
   // Perhitungan Rasio Penulangan //
   //////////////////////////////////
   RH1=AS1/(B*D);
   RHB=(0.75*(0.85*FC*BT1/FY)*(600./(600.+FS)))+RH1*FS1*FS1/FY;
   RHO = AS/(B*D);
   RMIN=1.4/FY;

   ///////////////////////////////////////////////
   // Menghitung Kendala Rasio Tulangan Balance //
   ///////////////////////////////////////////////
   kendala_rho_b=(RHO/RHB)-1.;
   if(kendala_rho_b<0)
   {
   	kendala_rho_b=0.;
   }

   ///////////////////////////////////////////////
   // Menghitung Kendala Rasio Tulangan Minimum //
   ///////////////////////////////////////////////
   kendala_rho_m=(RMIN/RHO)-1.;
   if(kendala_rho_m<0)
   {
   	kendala_rho_m=0;
   }

   //////////////////////////////////
   // Menghitung Kendala Rho Total //
   //////////////////////////////////
   kendala_rho=kendala_rho_b+kendala_rho_m;

   ////////////////////////////////
   // Momen Pada Balok Nmm -> Nm //
   ////////////////////////////////
   FMU=(teta*((0.85*FC*ARM*B)*(D-ARM/2.)+(AS1*FS1)*(D-DS)))/1000.;

   /////////////////////////////////////
   // Menghitung Kendala Momen Lentur //
   /////////////////////////////////////
   if(FMU>0)
   {
		kendala_M=((MU)/FMU)-1.;
   }
   else
   {
   	kendala_M=0.;
   }
   if(kendala_M<0)
   {
   	kendala_M=0.;
   }
}

/****************************************************/
/* FUNGSI UNTUK MENGHITUNG KENDALA PENULANGAN GESER */
/****************************************************/
void balok::sengkang_balok()
{
	DS=selimut_balok+(0.5*DIA1);
   D=H-DS;

   AV=0.25*pi*(pow(DIAS,2));

   /////////////////////////////////////////////////////////
   // Kuat Geser Nominal Yang Disumbangkan Tulangan Geser //
   /////////////////////////////////////////////////////////
	VC=(1./6.)*sqrt(FC)*B*D;
   VS=((VU/teta)-VC);

   ////////////////////////////////////////
   // Menghitung Jarak Sengkang Maksimal //
   ////////////////////////////////////////
   float Smak[4];
   if(VS<=0)
   {
   	VS=limit_nol;
   }
   Smak[0]=3.*AV*FYS/B;
   Smak[1]=(AV*FYS*D)/(VS);
  	Smak[2]=0.5*D;
   Smak[3]=600.;
   if(VS>((1./3.)*sqrt(FC)*B*D))
   {
   	Smak[2]=0.25*D;
   	Smak[3]=300.;
   }
   SmakS=Smak[0];

   ////////////////////////
   // Cari Yang Terkecil //
   ////////////////////////
   for (int sk=1;sk<4;sk++)
   {
   	if(SmakS>Smak[sk])
      {
      	SmakS=Smak[sk];
      }
   }

   Sref=SmakS;
   //////////////////////////////////////////////////////
   // Menghitung Kendala Jarak Antar Sengkang Maksimum //
   //////////////////////////////////////////////////////
   kendala_sb=((Jarak_S/SmakS)-1.);
   if(kendala_sb<0.)
   {
   	kendala_sb=0.;
   }
}

/**********************************************/
/* FUNGSI UNTUK MENGHITUNG LENDUTAN MAKSIMUM  */
/* BERDASARKAN SK SNI-T-15-1991-03 AYAT 3.2.5 */
/**********************************************/
void lendutan(int no_batang_l)
{
   //////////////////////////
   // Hitung Besaran Dasar //
   //////////////////////////
   n=2.E5/(E*1.E-6);

   fr=0.7*sqrt(FC); // (MPa atau N/mm^2)

   ////////////////////////
   // Letak Garis Netral //
   ////////////////////////
   AS=(0.25*pi*(pow(DIA1,2))*NL1);
   AS1=(0.25*pi*(pow(DIA2,2))*NL2);

   LGN=(n*AS/B)*(sqrt(1.+((2.*B*(H-selimut_balok))/(n*AS)))-1.);

   //////////////////////////////////////////////////////////////////
  	// Menentukan Momen Inersia Penampang Retak Transformasi (mm^4) //
   //////////////////////////////////////////////////////////////////
   Icr=(1./3.)*B*pow(LGN,3)+n*AS*pow(((H-selimut_balok)-LGN),2);

   //////////////////////////////////////////
   // Momen Inersia Penampang Kotor (mm^4) //
   //////////////////////////////////////////
   Ig=(1./12.)*B*pow(H,3);

   /////////////////////////////////////////////////////
   // Momen Pada Saat Timbul Retak Pertama Kali (Nmm) //
   /////////////////////////////////////////////////////
   Mcr=fr*Ig/(0.5*H);


   /////////////////////////////////////////////////////////////
   // Momen Inersia Efektif Untuk Perhitungan Lendutan (mm^4) //
   /////////////////////////////////////////////////////////////
   Ie=pow((Mcr/(MLAP[no_batang_l]*1.e3)),3)*Ig
     +(1.-pow((Mcr/(MLAP[no_batang_l]*1.e3)),3))*Icr;

   //////////////////////////////////
   // Menghitung Lendutan Seketika //
   //////////////////////////////////
   //////////////////////////////////////////////////////////
   // Lendutan Pada Balok (mm)                             //
   // Dari Buku : Reinforced Concrete Mechanics And Design //
   // Halaman : 355                                        //
   //////////////////////////////////////////////////////////
   LENDUTAN=(
     				(
                 	(5.*(pow(L*1000.,2)))
                   /(48.*(E*1.E-6)*Ie)
               )*
               (
                 	 fabs(MLAP[no_batang_l])*1000.
               )
            );


	//////////////////////////////////////////////////
   // Menghitung Lendutan Jangka Panjang           //
   // Menurut SK SNI-T-15-1991-03 pasal 3.2.5(2.5) //
   //////////////////////////////////////////////////
   Lambda=2./(1.+(50.*AS1/(B*H)));
   LENDUTAN=(1.+Lambda)*LENDUTAN;

   LENDUTAN_IJIN=(L*1000.)/180.;

   kendala_lendutan=((LENDUTAN/LENDUTAN_IJIN)-1.);
   if(kendala_lendutan<0)
   {
     	kendala_lendutan=0.;
   }
}

//---------------------------------------------------------------------//
