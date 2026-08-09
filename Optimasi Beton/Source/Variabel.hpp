/***********************************************************************/
/* FILE : VARIABEL.HPP                                                 */
/* PENDEKLARASIAN VARIABEL GLOBAL                                      */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1998)                              */
/***********************************************************************/

/******************/
/* MAKRO VARIABEL */
/******************/
#define pi 3.14			  // Phi lingkaran
#define teta 0.8			  // Faktor reduksi kekuatan
#define limit_nol 1.E-3   // Penetapan angka kecil mendekati nol
#define bj_besi 7850.     // berat jenis besi (kg/m^3)
#define MD 12             // Jumlah perpindahan untuk satu batang

/*****************/
/* VARIABEL UMUM */
/*****************/
char fi[20];   		// Input nama file generik tanpa ekstensi
char finput[20];  	// File *.inp untuk data umum struktur
char fsisi[20];		// File *.isd untuk data sisi penampang
char fdial[20];		// File *.idl untuk diameter tulangan utama
char fjtl[20];    	// File *.ijl untuk jumlah tulangan utama
char fdias[20];   	// File *.ids untuk diameter tulangan sengkang
char fjts[20];			// File *.ijs untuk jarak tulangan sengkang
char fbeban[20];		// File *.bbn untuk data beban
char fhistory[20];   // File *.his untuk mencatat riwayat optimasi
char foptimasi[20];	// File *.opt untuk keluaran hasil optimasi
char fstruktur[20];  // File *.str untuk hasil analisa struktur
char fkendala[20];   // File *.kdl untuk keluaran kendala struktur
char finformasi[20]; // File *.inf untuk keluaran informasi masukkan
char SubName[80];		// Informasi nama sub masukkan
char ISN[80];			// Informasi nama struktur

/**********************************************************/
/* VARIABEL MEKANIKA REKAYASA UNTUK STRUKTUR PORTAL RUANG */
/**********************************************************/
int NB; 			// Setengah lebar jalur matrik kekakuan
int M; 			// Jumlah batang
int NJ; 			// Jumlah titik kumpul
int NR;			// Jumlah Pengekang tumpuan
int NRJ; 		// Jumlah titik kumpul yang dikekang
int JRL[mak];	// Daftar pengekang titik kumpul
int T_K[mak];	// Daftar titik kumpul yang dikekang
int ND;			// Jumlah koordinat perpindahan untuk semua titik kumpul
int N; 			// Jumlah derajat kebebasan
int IA[mak];	// Notasi penunjuk nol atau tidaknya sudut alfa
int JJ[mak]; 	// Penunjuk untuk ujung j
int JK[mak]; 	// penunjuk untuk ujung k
int ID[mak]; 	// Indeks perpindahan untuk titik kumpul
int IR,IC; 		// Indeks baris dan kolom
int IM[mak]; 	// Indeks perpindahan batang
int LML[mak];	// Tabel batang yang dibebani

float E; 		// Modulus elastisitas
float G; 		// Modulus Puntir
float X[mak];  // Koordinat struktur pada arah x ( _ )     |y
float Y[mak];  // Koordinat struktur pada arah y ( | )     |___
float Z[mak]; 	// Koordinat struktur pada arah z ( / )   z/   x
float b[mak]; 	// Lebar penampang
float h[mak];	// Tinggi penampang
float AX[mak]; // Luas Penampang
float XI[mak]; // Konstanta puntir batang
float YI[mak]; // Momen inersia terhadap sumbu y batang
float ZI[mak]; // Momen inersia terhadap sumbu z batang
float EL[mak]; // Panjang penampang

float CX;      // Kosinus arah x
float Cy;      // Kosinus arah y
float CZ;		// Kosinus arah z
float CXZ;		// Penunjuk balok atau kolom
float XP[mak];	// Koordinat x dari titik P (m)
float YP[mak]; // Koordinat y dari titik P (m)
float ZP[mak]; // Koordinat z dari titik P (m)
float XPS;		// Koordinat xs dari titik P (m)
float YPS;		// Koordinat ys dari titik P (m)
float ZPS;		// Koordinat zs dari titik P (m)
float YPG; 		// Koordinat yg dari titik P (m)
float ZPG;		// Koordinat zg dari titik P (m)
float COSA; 	// Cosinus sudut alfa
float SINA;		// Sinus sudut alfa

float R11[mak];         // ~
float R12[mak];         //  |
float R13[mak];			//  |
float R21[mak];         //  |
float R22[mak];         //   -- Matrik Rotasi
float R23[mak];			//  |
float R31[mak];         //  |
float R32[mak];         //  |
float R33[mak];			// ~

float SM[13][13];		  	// Matrik kekakuan batang lokal
float SMRT[13][13]; 		// Hasil perkalian matrik SM dan matrik Rotasi
float SMS[mak][mak];		// Matrik kekakuan batang global
float SFF[mak][mak];    // Matrik kekakuan untuk perpindahan bebas
float DF[mak];				// Perpindahan titik dalam sumbu global (m)
float AJ[mak];				// Aksi beban pada titik kumpul dalam arah global
float AML[13][mak];		// Gaya ujung batang terkekang dalam arah lokal
float AE[mak];          // Beban titik kumpul ekivalen dalam arah global
float AC[mak];				// Beban titik kumpul gabungan dalam arah global
float DJ[mak];				// Perpindahan titik kumpul dalam arah global
float AMD[mak];			// Gaya ujung batang akibat perpindahan titik
float AM[mak][mak];		// Gaya ujung batang akhir
float AR[mak];				// Reaksi tumpuan dalam arah sumbu global
float W[mak]; 				// Beban Merata
float W_Balok[mak];     // Beban Berat Sendiri Balok
float P_Kolom[mak];		// Gaya Terpusat Kolom

float MTUM_KI[mak];		// Momen pada tumpuan kiri
float MTUM_KA[mak];		// Momen pada tumpuan kanan
float MLAP[mak];			// Momen lapangan
float GESER_KI[mak];		// Gaya geser sebelah kiri
float GESER_KA[mak];		// Gaya geser sebelah kanan

float MKX[mak];         // Momen kolom arah x
float MKY[mak];			// Momen kolom arah y
float PK[mak];				// Gaya aksial kolom
float GK[mak];				// Gaya geser kolom

/****************************/
/* VARIABEL BETON BERTULANG */
/****************************/

////////////////////////
// Data Masukkan Umum //
////////////////////////
float FC;  // Kuat desak karakteristik Beton (MPa)
float FY;  // Kuat tarik baja tulangan utama (MPa)
float FYS; // Kuat tarik baja tulangan sengkang (MPa)
float BT1; // Faktor reduksi tinggi blok tegangan ekivalen beton

/////////////////////////////
// Variabel Desain Sharing //
/////////////////////////////
int nsisi_B; // Jumlah data diskrit lebar balok
int nsisi_H; // Jumlah data diskrit tinggi balok
int nsisi_K; // Jumlah data diskrit sisi kolom

int nDIA;    // Jumlah data diskrit diameter
int nNL;     // Jumlah data diskrit jumlah diameter
int nDIAS;   // Jumlah data diskrit diameter sengkang
int nJS;     // Jumlah data diskrit jarak antar sengkang

float DIAS;			          // Diameter sengkang (mm)
float Jarak_S;		          // Jarak antar sengkang (mm)
float Sref;						 // Jarak antar sengkang yang dibutuhkan (mm)
float jarak_antar_tulangan; // Jarak antar tulangan (mm)                

float sisi_d_B[mak]; // Array berisi data diskrit lebar balok (mm)
float sisi_d_H[mak]; // Array berisi data diskrit tinggi balok (mm)
float sisi_d_K[mak]; // Array berisi data diskrit sisi kolom (mm)

float DIA_d[mak];  // Array berisi data diskrit diameter (mm)
float NL_d[mak];	 // Array berisi data diskrit jumlah tulangan
float DIAS_d[mak]; // Array berisi data diskrit diameter sengkang (mm)
float JS_d[mak];   // Array berisi data diskrit jarak antar sengkang (mm)

///////////////////////////
// Variabel Desain Balok //
///////////////////////////
float B;		 // Lebar balok (mm)
float H;		 // Tinggi balok (mm)
float DIA1;	 // Diameter tulangan tarik (mm)
float DIA2;	 // Diameter tulangan desak (mm)
float NL1;	 // Jumlah tulangan tarik (mm)
float NL2;	 // Jumlah tulangan desak (mm)

///////////////////////////
// Variabel Desain Kolom //
///////////////////////////
float sisi;	 // Sisi penampang kolom biaksial (mm)
float DIA;	 // Diameter tulangan (mm)
float N_DIA; // Jumlah tulangan

///////////////////////////
// Sharing Variabel Gaya //
///////////////////////////
float PU;	 // Gaya aksial ultimit (N)
float MU;	 // Momen ultimit (Nm)
float VU;	 // Gaya geser ultimit (N)
float VC;    // Gaya geser yang disumbangkan oleh beton (N)
float VS;	 // Gaya geser yang disumbangkan oleh tulangan geser (N)
float FMU;   // Gaya Momen Ultimit (Nm)
float FPU;	 // gaya Aksial Ultimit (N)

////////////////////////////
// Sharing Variabel Biaya //
////////////////////////////
float harga_beton;    // Harga beton (Rp./m^3)
float harga_besi;	    // Harga besi (Rp./kg)
float volume_beton;   // Volume beton yang digunakan (m^3)
float berat_besi;     // Berat besi yang digunakan (kg)
float berat_sengkang; // Berat sengkang yang digunakan (kg)
float L;				    // Panjang batang (m)

/////////////////////////////
// Sharing Variabel Elemen //
/////////////////////////////
float DS;	 // Jarak serat tarik terluar ke tulangan tarik (mm)
float D;		 // Jarak serat tekan terluar ke tulangan tarik (mm)
float AV;	 // Luas tulangan geser (mm^2)
float AVmin; // Luas tulangan geser minimum (mm^2)

///////////////////////////
// Variabel Khusus Balok //
///////////////////////////
float selimut_balok;	// Tebal selimut pada balok

float DIA1lap;			// Diameter tulangan tarik pada daerah lapangan
float NL1lap; 			// Jumlah tulangan tarik pada daerah lapangan
float DIA2lap;			// Diameter tulangan desak pada daerah lapangan
float NL2lap; 			// Jumlah tulangan desak pada daerah lapangan

float DIA1tum;			// Diameter tulangan tarik pada daerah tumpuan
float NL1tum; 			// Jumlah tulangan tarik pada daerah tumpuan
float DIA2tum;			// Diameter tulangan desak pada daerah tumpuan
float NL2tum; 			// Jumlah tulangan desak pada daerah tumpuan

float AS;     			// Luas tulangan tarik
float AS1;	  			// Luas tulangan desak
float HMIN;          // Tinggi Minimum
float n;   				// Rasio Es/Ec
float fr;            // Modulus Keruntuhan lentur dari beton (MPa)
float LGN;           // Letak Garis Netral (mm)
float Icr;				// Momen Inersia Penampang Retak (mm^4)
float Ig;            // Momen Inersia Penampang Kotor (mm^4)
float Ie;            // Momen Inersia Efektif (mm^4)
float Mcr;				// Momen Retak (Nmm);
float LENDUTAN;      // Lendutan pada tengah bentang (mm)
float LENDUTAN_IJIN; // Lendutan ijin (mm);
float Lambda;        // Faktor pengali lendutan jangka panjang


///////////////////////////
// Variabel Khusus Kolom //
///////////////////////////
float MUX;					// Momen arah x ultimit (Nm)
float MUY;					// Momen arah y ultimit (Nm)
float selimut_kolom;		// Tebal selimut pada kolom
float _K;               // Faktor kelangsingan

/*******************************************************/
/* VARIABEL OPTIMASI DENGAN METODA FLEXIBLE POLYHEDRON */
/*******************************************************/
int jum_susut;          // Jumlah penyusutan berturut
int fak_plus;           // Faktor penambah (JSTD=JVD+fak_plus)
int fak_kali;           // Faktor Pengali (JSTD=JVD*fak_kali)
int no_struktur;			// Nomor Struktur
int JVD;						// Jumlah variabel desain
int JSTD;					// Jumlah struktur
int j_iterasi_mak;      // Jumlah iterasi maksimum
int nvm[mak];				// Batas atas variabel diskrit normal
int nvk[mak];				// Batas atas variabel diskrit kolom
int nvb[mak];				// Batas atas variabel diskrit balok
int iterasi_var;        // Jumlah iterasi untuk melangkah
int var_b[mak][mak];    // Nomor variabel balok diskrit yang digunakan
int var_k[mak][mak];		// Nomor variabel kolom diskrit yang digunakan
int tanda_arah_b[mak];	// Tanda arah-arah penelusuran bagian balok
int tanda_arah_k[mak];	// Tanda arah-arah penelusuran bagian kolom
int var_b_jelek[mak];	// Koordinat variabel jelek bagian balok
int var_k_jelek[mak];	// Koordinat variabel jelek bagian kolom
int arah[mak];				// Koordinat arah penelusuran
int no_TS_terjauh;      // Nomor arah yang paling jauh
int varnew_asli[mak];	// Variabel terjelek hasil penggandaan
int varnew[mak];			// Variabel baru bertipe integer
int var_b_cb[mak];      // Variabel desain balok baru
int var_k_cb[mak];      // Variabel desain kolom baru
int var_b_cb_best[mak]; // Variabel desain balok baru terbaik
int var_k_cb_best[mak]; // Variabel desain kolom baru terbaik
int no_balok[mak]; 	   // Identifikasi nomor balok dengan nomor batang
int no_kolom[mak]; 		// Identifikasi nomor kolom dengan nomor batang
int jum_balok;     		// Jumlah balok pada struktur
int jum_kolom;     		// Jumlah kolom pada struktur
int js_balok;				// Nomor balok (Pembantu)
int js_kolom;				// Nomor kolom (Pembantu)
int lompat;					// Identifikasi untuk melompat dari loop

float patok_fit;        // Variabel Patokan Fitness
float finalti;				// Faktor finalti untuk pelanggaran
float fitstr[mak];		// Fitnes struktur
float fitcb;            // Fitness baru
float fitcb_best;       // Fitnes baru yang terbaik

float kendalastr[mak];	// Kendala struktur
float hargastr[mak];		// Harga struktur (Rp.)
float kendala;          // Kendala
float kendalasa;			// Kendala pada saat struktur awal
float harga;            // Harga (Rp.)
float XM_b[mak];        // Koordinat titik Midle bagian balok
float XM_k[mak];			// Koordinat titik Midle bagian kolom
float XS_b[mak];			// Arah-arah penelusuran bagian balok
float XS_k[mak];			// Arah-arah penelusuran bagian kolom
float TM[mak];          // Koordinat titik Midle
float TS[mak];				// Koordinat titik Search
float varplus[mak];     // Variabel penambahan
float varnew_f[mak];		// Variabel baru bertipe float

/////////////////////////////////////
// Variabel Kendala Kolom Biaksial //
/////////////////////////////////////
float kendala_gaya; 			 // Kendala gaya pada material
float kendala_po;   			 // Gaya tekan maksimum akibat eksentrisitas
float kendala_pn;   			 // Kendala gaya tekan
float kendala_mn;   			 // Kendala momen

float kendala_r;				 // Kendala rasio penulangan
float kendala_r_min;			 // Rasio tulangan minimum
float kendala_r_mak;			 // Rasio tulangan maksimum

float kendala_sengkang;		 // Kendala sengkang
float kendala_tul;			 // Kendala jumlah dan jarak tulangan
float kendala_kelangsingan; // Kendala kelangsingan

////////////////////////////
// Variabel Kendala Balok //
////////////////////////////
float kendala_rho;	// Besarnya kendala akibat rasio tulangan
float kendala_rho_b;	// Besarnya kendala akibat rasio tulangan balance
float kendala_rho_m;	// Besarnya kendala akibat rasio tulangan minimum
float kendala_sb;    // Besarnya kendala akibat kebutuhan sengkang
float kendala_M;    	// Besarnya kendala akibat momen
float kendala_lendutan;  // Besarnya kendala akibat tinggi minimum

//---------------------------------------------------------------------//
