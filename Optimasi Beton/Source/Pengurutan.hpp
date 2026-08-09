/***********************************************************************/
/* FILE : PENGURUTAN.HPP                                               */
/* SUBPROGRAM UNTUK MENGURUTKAN DATA BERDASARKAN FITNESS               */
/* DENGAN BANTUAN METODA BUBBLE SORT                                   */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/
void sort(float nilai1[],float nilai2[],float nilai3[]
			,int urut_var_b[][mak],int urut_var_k[][mak]
         ,int n_array)
{
   ////////////////////////
   // Metode Bubble Sort //
   ////////////////////////

   /////////////////////////////////////////////////////////////////
   // Nilai 1 Untuk Fitness                                       //
   // Nilai 2 Untuk Kendala                                       //
   // Nilai 3 Untuk Harga                                         //
   // urut_var_b Untuk Variabel Balok                             //
   // urut_var_k Untuk Variabel Kolom                             //
   // n_array Adalah Jumlah Titik Yang Akan Diurutkan Fitnessnya  //
   /////////////////////////////////////////////////////////////////
   double dummy1=0.;
   float dummy2=0.;
   float dummy3=0.;
   int dummyvarb[mak];
   int dummyvark[mak];
   for (int dum1=0;dum1<JSTD;dum1++)
   {
      dummyvarb[dum1]=0;
      dummyvark[dum1]=0;
   }

   ////////////////////////////////////////////////
   // Pemeriksaaan Dimulai Dari Nilai Terkecil+1 //
   ////////////////////////////////////////////////
	for(int isort=1;isort<n_array;++isort)
   {
      ////////////////////////////////////////////////
      // Pembandingan Kepada Nilai-Nilai Sebelumnya //
      ////////////////////////////////////////////////
   	for(int jsort=0;jsort<=isort;++jsort)
      {
         ////////////////////////////////////////////////////////////////
         // Bila Nilai Sekarang Lebih Kecil Daripada Nilai Sebelumnya  //
         // Maka Dilakukan Penukaran Posisi Ke Tempat Nilai Sebelumnya //
         ////////////////////////////////////////////////////////////////
        	if(nilai1[isort]<nilai1[jsort])
      	{
            ////////////////////////////////////////////////
            // Pengisian Nilai Sekarang Ke Variabel Dummy //
            ////////////////////////////////////////////////
      		dummy1=nilai1[isort];
            dummy2=nilai2[isort];
            dummy3=nilai3[isort];
            for(int dvb=0;dvb<jum_balok;dvb++)
   			{
               for(int dnv_b=0;dnv_b<12;dnv_b++)
      			{
        				dummyvarb[dnv_b+(12*dvb)]
                  	=urut_var_b[isort][dnv_b+(12*dvb)];
      			}
            }
            for(int dvk=0;dvk<jum_kolom;dvk++)
            {
               for(int dnv_k=0;dnv_k<5;dnv_k++)
      			{
        				dummyvark[dnv_k+(5*dvk)]
                  	=urut_var_k[isort][dnv_k+(5*dvk)];
      			}
            }

            /////////////////////////////////////////////////////
            // Pemindahan Nilai Yang Akan Digantikan Posisinya //
            // Ke Posisi Sekarang                              //
            /////////////////////////////////////////////////////
            nilai1[isort]=nilai1[jsort];
            nilai2[isort]=nilai2[jsort];
            nilai3[isort]=nilai3[jsort];
            for(int dvb=0;dvb<jum_balok;dvb++)
   			{
            	for(int dnv_b=0;dnv_b<12;dnv_b++)
      			{
	            	urut_var_b[isort][dnv_b+(12*dvb)]
                  	=urut_var_b[jsort][dnv_b+(12*dvb)];
               }
            }
            for(int dvk=0;dvk<jum_kolom;dvk++)
            {
               for(int dnv_k=0;dnv_k<5;dnv_k++)
      			{
            		urut_var_k[isort][dnv_k+(5*dvk)]
                  	=urut_var_k[jsort][dnv_k+(5*dvk)];
               }
            }

            /////////////////////////////////////////////////////////
            // Pemindahan Nilai Sekarang Ke Posisi Yang Digantikan //
            /////////////////////////////////////////////////////////
            nilai1[jsort]=dummy1;
            nilai2[jsort]=dummy2;
            nilai3[jsort]=dummy3;
            for(int dvb=0;dvb<jum_balok;dvb++)
   			{
               for(int dnv_b=0;dnv_b<12;dnv_b++)
      			{
	            	urut_var_b[jsort][dnv_b+(12*dvb)]
                  	=dummyvarb[dnv_b+(12*dvb)];
               }
            }
            for(int dvk=0;dvk<jum_kolom;dvk++)
            {
               for(int dnv_k=0;dnv_k<5;dnv_k++)
      			{
            		urut_var_k[jsort][dnv_k+(5*dvk)]
                  	=dummyvark[dnv_k+(5*dvk)];
               }
            }

            ///////////////////////////////////
	         // Proses Penukaran Selesai      //
   	      // Melanjutkan Nilai Selanjutnya //
      	   ///////////////////////////////////
         }
      }
   }

   ///////////////////////////////////////////////////////////
   // Sesudah Diurutkan Nilai Terjelek Terdapat Pada Array 0 //
   // Nilai Terbaik Terdapat Pada (n_array-1)              //
   ///////////////////////////////////////////////////////////
}

//---------------------------------------------------------------------//
