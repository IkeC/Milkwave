// prefs.h

class CPrefs {
public:
  IMMDevice* m_pMMDevice;
  bool m_bIsRenderDevice = true;
  HMMIO m_hFile;
  bool m_bInt16;
  PWAVEFORMATEX m_pwfx;
  LPCWSTR m_szFilename;
  std::wstring m_szAudioDeviceDisplayName;
  int m_nAudioDeviceRequesType = 0; // 0: Undefined, 1: Capture (in), 2: Render (out)

  // set hr to S_FALSE to abort but return success
  CPrefs(int argc, LPCWSTR argv[], HRESULT& hr, int nAudioDeviceRequesType);
  ~CPrefs();

};
