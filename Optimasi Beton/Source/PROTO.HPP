/***********************************************************************/
/* FILE : PROTO.HPP                                                    */
/* PROTOTYPE OPTIMASI STRUKTUR BETON BERTULANG PADA PORTAL RUANG       */
/* DITULIS OLEH YOHAN NAFTALI (JUNI 1999)                              */
/***********************************************************************/

/**************************/
/* PROTOTYPE PROGRAM UMUM */
/**************************/
void menu_utama();
void about();
void input_data();
void input_data_umum();
void input_data_diskrit();

/***********************************************************************/
/* PROTOTYPE PROGRAM MEKANIKA REKAYASA STRUKTUR DENGAN METODA KEKAKUAN */
/***********************************************************************/
void baca_data();
void load_data();
void kosong_beban();
void baca_beban();
void struktur();
void inersia();
void kekakuan_batang();
void beban();
void berat_sendiri();
void isi_matrik_kekakuan(int imk);
void indeks_batang(int ib);
void periksa_batang(int b);
void banfac(int N,int NB,float A_SFF[][mak]);
void bansol(int N,int NB,float U_SFF[][mak],float B_AC[],float X_DF[]);
void hasil();
void output_parameter_struktural();
void output_koordinat_titik_kumpul();
void output_informasi_batang();
void output_pengekang_titik_kumpul();
void output_beban_batang();
void output_beban_titik();

/*************************************/
/* PROTOTYPE PROGRAM BETON BERTULANG */
/*************************************/
void isi_elemen_balok(int no_el_balok);
void elemen_lapangan(int no_el_balok);
void elemen_tumpuan(int no_el_balok);
void isi_elemen_kolom(int no_el_kolom);
void lendutan(int no_batang_l);

/****************************************************************/
/* PROTOTYPE PROGRAM OPTIMASI DENGAN METODA FLEXIBLE POLYHEDRON */
/****************************************************************/
float Kendala_Harga(int var_b_nya[],int var_k_nya[]);
float isi(int no_data,float kelompok_data[]);
void optimasi();
void load_batas_atas();
void acak_variabel();
void randomisasi();
void cari_struktur_awal();    
void cari_baru();
void periksa_batas();
void penelusuran();
void ganti_baru();
void penyusutan();
void normalisasi_float(float var_nor[],float var_bv[],float var_kv[]);
void normalisasi_int(int var_nor[],int var_bv[],int var_kv[]);
void unnormalisasi(float var_nor[],float var_bv[],float var_kv[]);
void cetak_akhir();
void sort(float nilai1[],float nilai2[],float nilai3[]
          ,int urut_var_b[][mak],int urut_var_k[][mak],int n_array);
int konversi(float bil_asli);

//----------------------------------------------------------------------//
