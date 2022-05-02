/*
  This source is extracted from M5Unified's example(Bluetooth_with_ESP32A2DP.ino).
  https://github.com/m5stack/M5Unified
 */
/// need ESP32-A2DP library. ( URL : https://github.com/pschatzmann/ESP32-A2DP/ )
#include <BluetoothA2DPSink.h>
#include <M5Unified.h>



typedef void (*hvtEventCallback)(int expression);

class BluetoothA2DPSink_M5Speaker : public BluetoothA2DPSink
{
public:
  BluetoothA2DPSink_M5Speaker(m5::Speaker_Class* m5sound, uint8_t virtual_channel = 0)
  : BluetoothA2DPSink()
  {
    is_i2s_output = false; // I2S control by BluetoothA2DPSink is not required.
    _virtual_channel = virtual_channel;
  }

  // get rawdata buffer for FFT.
  const int16_t* getBuffer(void) const { return _tri_buf[_export_index]; }

  const char* getMetaData(size_t id) { _meta_bits &= ~(1<<id); return (id < metatext_num) ? _meta_text[id] : nullptr; }
  

  uint8_t getMetaUpdateInfo(void) const { return _meta_bits; }

  void setHvtEventCallback(hvtEventCallback cb) { _hvt_evt_cb = cb; }

  void clear(void)
  {
    for (int i = 0; i < 3; ++i)
    {
      if (_tri_buf[i]) { memset(_tri_buf[i], 0, _tri_buf_size[i]); }
    }
  }

  static constexpr size_t metatext_size = 80;
  static constexpr size_t metatext_num = 3;

protected:
  int16_t* _tri_buf[3] = { nullptr, nullptr, nullptr };
  size_t _tri_buf_size[3] = { 0, 0, 0 };
  size_t _tri_index = 0;
  size_t _export_index = 0;
  char _meta_text[metatext_num][metatext_size];
  uint8_t _meta_bits = 0;
  bool _sing_happy = true;
  size_t _sample_rate = 48000;
  int _avatar_expression = 5; // m5stack-avatarの表情取得用
  hvtEventCallback _hvt_evt_cb = nullptr;
  uint8_t _virtual_channel = 0;

  void clearMetaData(void)
  {
    for (int i = 0; i < metatext_num; ++i)
    {
      _meta_text[i][0] = 0;
    }
    _meta_bits = (1<<metatext_num)-1;
  }

  void av_hdl_a2d_evt(uint16_t event, void *p_param) override
  {
    esp_a2d_cb_param_t* a2d = (esp_a2d_cb_param_t *)(p_param);

    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
      if (ESP_A2D_CONNECTION_STATE_CONNECTED == a2d->conn_stat.state)
      { // 接続
        _avatar_expression = 5; //Neutral
        //avatar.setExpression(Expression::Neutral);

      }
      else
      if (ESP_A2D_CONNECTION_STATE_DISCONNECTED == a2d->conn_stat.state)
      { // 切断
        _avatar_expression = 2;
        //avatar.setExpression(Expression::Sad);
      }
      break;

    case ESP_A2D_AUDIO_STATE_EVT:
      if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state)
      { // 再生
        if (_sing_happy) {
          _avatar_expression = 0;
          // avatar.setExpression(Expression::Happy);
        } else {
          _avatar_expression = 5;
          //avatar.setExpression(Expression::Neutral);
        }
        _sing_happy = !_sing_happy;
      } else
      if ( ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND == a2d->audio_stat.state
        || ESP_A2D_AUDIO_STATE_STOPPED        == a2d->audio_stat.state )
      { // 停止
        _avatar_expression = 4;
        // avatar.setExpression(Expression::Sleepy);
        clearMetaData();
        clear();
      }
      break;

    case ESP_A2D_AUDIO_CFG_EVT:
      {
        esp_a2d_cb_param_t *a2d = (esp_a2d_cb_param_t *)(p_param);
        size_t tmp = a2d->audio_cfg.mcc.cie.sbc[0];
        size_t rate = 16000;
        if (     tmp & (1 << 6)) { rate = 32000; }
        else if (tmp & (1 << 5)) { rate = 44100; }
        else if (tmp & (1 << 4)) { rate = 48000; }
        _sample_rate = rate;
      }
      break;

    default:
      break;
    }
    
    if (_hvt_evt_cb != nullptr) {
        _hvt_evt_cb(_avatar_expression);
    }

    BluetoothA2DPSink::av_hdl_a2d_evt(event, p_param);
  }

  void av_hdl_avrc_evt(uint16_t event, void *p_param) override
  {
    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(p_param);

    switch (event)
    {
    case ESP_AVRC_CT_METADATA_RSP_EVT:
      for (size_t i = 0; i < metatext_num; ++i)
      {
        if (0 == (rc->meta_rsp.attr_id & (1 << i))) { continue; }
        strncpy(_meta_text[i], (char*)(rc->meta_rsp.attr_text), metatext_size);
        _meta_bits |= rc->meta_rsp.attr_id;
        break;
      }
      break;

    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
      break;

    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
      break;

    default:
      break;
    }

    BluetoothA2DPSink::av_hdl_avrc_evt(event, p_param);
  }

  int16_t* get_next_buf(const uint8_t* src_data, uint32_t len)
  {
    size_t tri = _tri_index < 2 ? _tri_index + 1 : 0;
    if (_tri_buf_size[tri] < len)
    {
      _tri_buf_size[tri] = len;
      if (_tri_buf[tri] != nullptr) { heap_caps_free(_tri_buf[tri]); }
      auto tmp = (int16_t*)heap_caps_malloc(len, MALLOC_CAP_8BIT);
      _tri_buf[tri] = tmp;
      if (tmp == nullptr)
      {
        _tri_buf_size[tri] = 0;
        return nullptr;
      }
    }
    memcpy(_tri_buf[tri], src_data, len);
    _tri_index = tri;
    return _tri_buf[tri];
  }

  void audio_data_callback(const uint8_t *data, uint32_t length) override
  {
    // Reduce memory requirements by dividing the received data into the first and second halves.
    length >>= 1;
    M5.Speaker.playRaw(get_next_buf( data        , length), length >> 1, _sample_rate, true, 1, _virtual_channel);
    M5.Speaker.playRaw(get_next_buf(&data[length], length), length >> 1, _sample_rate, true, 1, _virtual_channel);
    _export_index = _tri_index;
  }
};


#define FFT_SIZE 256
class fft_t
{
  float _wr[FFT_SIZE + 1];
  float _wi[FFT_SIZE + 1];
  float _fr[FFT_SIZE + 1];
  float _fi[FFT_SIZE + 1];
  uint16_t _br[FFT_SIZE + 1];
  size_t _ie;

public:
  fft_t(void)
  {
#ifndef M_PI
#define M_PI 3.141592653
#endif
    _ie = logf( (float)FFT_SIZE ) / log(2.0) + 0.5;
    static constexpr float omega = 2.0f * M_PI / FFT_SIZE;
    static constexpr int s4 = FFT_SIZE / 4;
    static constexpr int s2 = FFT_SIZE / 2;
    for ( int i = 1 ; i < s4 ; ++i)
    {
    float f = cosf(omega * i);
      _wi[s4 + i] = f;
      _wi[s4 - i] = f;
      _wr[     i] = f;
      _wr[s2 - i] = -f;
    }
    _wi[s4] = _wr[0] = 1;

    size_t je = 1;
    _br[0] = 0;
    _br[1] = FFT_SIZE / 2;
    for ( size_t i = 0 ; i < _ie - 1 ; ++i )
    {
      _br[ je << 1 ] = _br[ je ] >> 1;
      je = je << 1;
      for ( size_t j = 1 ; j < je ; ++j )
      {
        _br[je + j] = _br[je] + _br[j];
      }
    }
  }

  void exec(const int16_t* in)
  {
    memset(_fi, 0, sizeof(_fi));
    for ( size_t j = 0 ; j < FFT_SIZE / 2 ; ++j )
    {
      float basej = 0.25 * (1.0-_wr[j]);
      size_t r = FFT_SIZE - j - 1;

      /// perform han window and stereo to mono convert.
      _fr[_br[j]] = basej * (in[j * 2] + in[j * 2 + 1]);
      _fr[_br[r]] = basej * (in[r * 2] + in[r * 2 + 1]);
    }

    size_t s = 1;
    size_t i = 0;
    do
    {
      size_t ke = s;
      s <<= 1;
      size_t je = FFT_SIZE / s;
      size_t j = 0;
      do
      {
        size_t k = 0;
        do
        {
          size_t l = s * j + k;
          size_t m = ke * (2 * j + 1) + k;
          size_t p = je * k;
          float Wxmr = _fr[m] * _wr[p] + _fi[m] * _wi[p];
          float Wxmi = _fi[m] * _wr[p] - _fr[m] * _wi[p];
          _fr[m] = _fr[l] - Wxmr;
          _fi[m] = _fi[l] - Wxmi;
          _fr[l] += Wxmr;
          _fi[l] += Wxmi;
        } while ( ++k < ke) ;
      } while ( ++j < je );
    } while ( ++i < _ie );
  }

  uint32_t get(size_t index)
  {
    return (index < FFT_SIZE / 2) ? (uint32_t)sqrtf(_fr[ index ] * _fr[ index ] + _fi[ index ] * _fi[ index ]) : 0u;
  }
};
