

#include "stdafx.h"
#include "CharLobby.h"

void CCharLobby::OnSignal_Community(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	switch((LONG)hSignal)
	{
	case CCharLobby::CHARLOBBY_NLS_GET_FRIEND_LIST_INFO:
		{
			ProcessNFFriendList((LPXBUF)wParam);
			break;
		}
	case CCharLobby::CHARLOBBY_PROCESS_NF_FRIEND_APPLICATION:
		{
			ProcessNFFriendApplication((LPXBUF)wParam);
			break;
		}
	case CCharLobby::CHARLOBBY_PROCESS_NF_FRIEND_ACCEPT:
		{
			ProcessNFFriendAccept((LPXBUF)wParam);
			break;
		}
	case CCharLobby::CHARLOBBY_PROCESS_NF_FRIEND_AUTO_ACCEPT:
		{
			ProcessNFFriendAutoAccept((LPXBUF)wParam);
			break;
		}
	case CCharLobby::CHARLOBBY_PROCESS_FOLLOW_USER:
		{
			ProcessFollowUser((LPXBUF)wParam);
			break;
		}
	case CCharLobby::CHARLOBBY_PROCESS_NF_LETTER_NEW:
		{
			ProcessNFLetterNew((LPXBUF)wParam);
			break;
		}
	case CHARLOBBY_NOTIFY_ACCEPT_FRIEND_FROM_CHS:
		{
			ProcessAcceptFriendFromCHS((LPXBUF)wParam);
			break;
		}
	case CHARLOBBY_NOTIFY_NEW_LETTER_FROM_NGS:
		{
			ProcessNewLetterFromNGS((LPXBUF)wParam);
			break;
		}
	case CHARLOBBY_NOTIFY_ACCEPT_FRIEND_FROM_NGS:
		{
			ProcessAcceptFriendFromNGS((LPXBUF)wParam);
			break;
		}
	case CHARLOBBY_ADDFRIEND_NOTIFY:
		{
			ProcessAddFriendFromNGS((LPXBUF)wParam);
			break;
		}
	default:
		{
			TLock lo(this);
			TBase::OnSignal( hSignal, wParam, lParam );
		}
	}
}

void CCharLobby::ProcessNFFriendList(LPXBUF buf)
{
	TLock lo(this);
	MsgNLSCLI_AnsLocation msg;
	if(!::LLoad(msg, buf))
		return;
	m_pCharLobbyCxt->ProcessNFFriendList(msg);
}

void CCharLobby::ProcessNFFriendApplication(LPXBUF buf)
{
	TLock lo(this);
	MsgNLSCLI_AnsLocation msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessNFFriendApplication(msg);
}

void CCharLobby::ProcessNFFriendAccept(LPXBUF buf)
{
	TLock lo(this);
	MsgNLSCLI_AnsLocation msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessNFFriendAccept(msg);
}

void CCharLobby::ProcessNFFriendAutoAccept(LPXBUF buf)
{
	TLock lo(this);
	MsgNLSCLI_AnsLocation msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessNFFriendAutoAccept(msg);
}

void CCharLobby::ProcessFollowUser(LPXBUF buf)
{
	TLock lo(this);
	MsgNLSCLI_AnsLocation msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessFollowUser(msg);
}

void CCharLobby::ProcessNFLetterNew(LPXBUF buf)
{
	TLock lo(this);
	MsgNLSCLI_AnsLocation msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessNFLetterNew(msg);
}

void CCharLobby::ProcessAcceptFriendFromCHS(LPXBUF buf)
{
	TLock lo(this);
	MsgCHSNCS_NtfNFFriendAccept msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessAcceptFriendFromCHS(msg);
}

void CCharLobby::ProcessNewLetterFromNGS(LPXBUF buf)
{
	TLock lo(this);
	MsgNGSNCS_NtfNFLetterReceive msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessNewLetterFromNGS(msg);
}

void CCharLobby::ProcessAcceptFriendFromNGS(LPXBUF buf)
{
	TLock lo(this);
	MsgNGSNCS_NtfNFFriendAccept msg;
	if(!::LLoad(msg, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessAcceptFriendFromNGS(msg);
}

void CCharLobby::ProcessAddFriendFromNGS(LPXBUF buf)
{
	TLock lo(this);
	MsgNGSNCS_NtfNFFriendAdd ntf;
	if(!::LLoad(ntf, (LPXBUF)buf))
		return;
	m_pCharLobbyCxt->ProcessAddFriendFromNGS(ntf);
}
