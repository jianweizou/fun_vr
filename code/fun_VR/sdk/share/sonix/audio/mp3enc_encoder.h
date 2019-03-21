#ifndef MP3ENC_ENCODER_H
#define MP3ENC_ENCODER_H







typedef struct  bit_stream_struc {
    unsigned char *data;
    int         data_size;
    int         data_position;
    unsigned int cache;
    int         cache_bits;
		int         err;
} bitstream_t;
typedef struct {
    int channels;
    int samplerate;
} priv_MP3Enc_wave_t;
typedef struct {
    int    version;
    int    layer;
    int    granules_per_frame;
    int    mode;
    int    bitr;
    int    emph;
    int    padding;
    int    bits_per_frame;
    int    bits_per_slot;
    double frac_slots_per_frame;
    double slot_lag;
    int    whole_slots_per_frame;
    int    bitrate_index;
    int    samplerate_index;
    int    crc;
    int    ext;
    int    mode_ext;
    int    copyright;
    int    original;
} priv_MP3Enc_mpeg_t;
typedef struct {
  int *xr;
  int xrsq[576];
  int xrabs[576];
  int xrmax;
  int en_tot[2];
  int en[2][21];
  int xm[2][21];
  int xrmaxl[2];
} l3loop_t;
typedef struct {
  int off[2];
  int x[2][512];
} subband_t; 
typedef struct {
  unsigned part2_3_length;
  unsigned big_values;
  unsigned count1;
  unsigned global_gain;
  unsigned scalefac_compress;
  unsigned table_select[3];
  unsigned region0_count;
  unsigned region1_count;
  unsigned preflag;
  unsigned scalefac_scale;
  unsigned count1table_select;
  unsigned part2_length;
  unsigned sfb_lmax;
  unsigned address1;
  unsigned address2;
  unsigned address3;
  int quantizerStepSize;
  unsigned slen[4];
} gr_info;
typedef struct {
    unsigned private_bits;
    int resvDrain;
    unsigned scfsi[2][4];
    struct {
        struct {
            gr_info tt;
        } ch[2];
    } gr[2];
} MP3Enc_side_info_t;
typedef struct {
    double  l[2][2][21];
} MP3Enc_psy_ratio_t;
typedef struct {
    int l[2][2][22];
    int s[2][2][13][3];
} MP3Enc_scalefac_t;
typedef struct MP3Enc_global_flags {
  priv_MP3Enc_wave_t    wave;
  priv_MP3Enc_mpeg_t    mpeg;
  bitstream_t    bs;
  MP3Enc_side_info_t side_info;
  int            sideinfo_len;
  int            mean_bits;
  MP3Enc_psy_ratio_t ratio;
  MP3Enc_scalefac_t  scalefactor;
  short       *buffer[2];
  double         pe[2][2];
  int            l3_enc[2][2][576];
  int        l3_sb_sample[2][2+1][18][32];
  int        mdct_freq[2][2][576];
  int            ResvSize;
  int            ResvMax;
  l3loop_t       l3loop;
  subband_t      subband;
  int interleave;
  int outbytes;
  int err;
  int bytes_per_pass;
  int swfft;
} SonixMP3Enc_encoder;





int MP3Enc_initialise_all(SonixMP3Enc_encoder* s, int freq, int bitr, int stereo_mode, int emph, int copyright, int original, int interleave, int outbuf_size);
int MP3Enc_encode(SonixMP3Enc_encoder* config, short *data, unsigned char* outbuf);
void MP3Enc_flush(SonixMP3Enc_encoder* s);








#endif
