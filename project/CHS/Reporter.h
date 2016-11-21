//
// Reporter.h
//

#ifndef Reporter_h
#define Reporter_h

///////////////////////////////////////////////////////////////////////////////////
// CReporter

class CReporter : public IXObject
{
	IMPLEMENT_TISAFEREFCNT(CReporter)
private:
	CReporter(const CReporter&);
public:
	CReporter();
	virtual ~CReporter();
public:
	virtual BOOL RunReport();
	virtual BOOL StopReport();
	BOOL ChangeInterval(DWORD dwDue, DWORD dwPeriod);
	void SetAllDiffFlag();
	// inherited
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);//BOOL bFired);			// TIMER..

	// implementation
protected:
	void OnSendCHSInfo();

protected:
	GXSigTimer m_timerCHSInfo;	
};


extern CReporter theReporter;

#endif //!Reporter_h
