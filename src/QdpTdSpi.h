//
// Created by quant on 6/8/16.
//

#ifndef QUANT_CTP_QDP_TDSPI_H
#define QUANT_CTP_QDP_TDSPI_H

#include <map>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <fstream>
#include <iostream>
using namespace std;

#include "QdpFtdcTraderApi.h"
#include "QdpFtdcUserApiStruct.h"
#ifndef NULL
#define NULL 0
#endif

using std::map;
using std::string;

class QdpTdSpi :public CQdpFtdcTraderSpi{
public:

	//增加毫秒
	void timeraddMS(struct timeval *a, int ms);

    //构造函数
    QdpTdSpi(CQdpFtdcTraderApi *tdapi);

	//建立连接
	void Connect(char *frontAddress);

    //当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    void OnFrontConnected();

	//登录
	void Login(char *BrokerID, char *UserID, char *Password);


    ///登录请求响应
    void OnRspUserLogin(CQdpFtdcRspUserLoginField *pRspUserLogin,
                        CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//登出
	void Logout(char *BrokerID, char *UserID);

    ///登出请求响应
	void OnRspUserLogout(CQdpFtdcRspUserLoginField *pUserLogout,
                         CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//等待线程结束
	void Join();

	//查询结算信息确认
	void QrySettlementInfoConfirm(char *BrokerID, char *InvestorID, int nRequestID);

	//查询交易所
	void QryExchange();

	//响应查询交易所
	void OnRspQryExchange(CQdpFtdcRspExchangeField *pExchange, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询合约
	void QryInstrument(string exchangeid = "", string instrumentid = "");

	//响应查询合约
	void OnRspQryInstrument(CQdpFtdcRspInstrumentField *pInstrument, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///合约交易状态通知
	void OnRtnInstrumentStatus(CQdpFtdcInstrumentStatusField *pInstrumentStatus);

	//查询报单
	void QryOrder();

	//响应查询报单;
	void OnRspQryOrder(CQdpFtdcOrderField *pOrder, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询投资者
	void QryInvestor();

	//查询投资者响应
	void OnRspQryInvestorAccount(CQdpFtdcRspInvestorAccountField *pInvestor, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//查询投资者持仓
	void QryInvestorPosition();
	
	//查询投资者持仓明细
	void QryInvestorPositionDetail();
	
	//查询成交单
	void QryTrade();

	//查询成交单响应
	void OnRspQryTrade(CQdpFtdcTradeField *pTrade, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    //下单
	void OrderInsert(char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef, string insert_time = "");

	void OrderInsert(CQdpFtdcInputOrderField *pInputOrder);

	void InitTestpInputOrder(char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef, string insert_time = "");

	//下单响应
	void OnRtnOrder(CQdpFtdcOrderField *pOrder);

	///成交回报
	void OnRtnTrade(CQdpFtdcTradeField *pTrade);

	//下单错误响应
	void OnErrRtnOrderInsert(CQdpFtdcInputOrderField *pInputOrder, CQdpFtdcRspInfoField *pRspInfo);

	///报单录入请求响应
	void OnRspOrderInsert(CQdpFtdcInputOrderField *pInputOrder, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//撤单
	void OrderAction(string ExchangeID, string UserOrderLocalID, string OrderSysID, string UserOrderActionLocalID, string action_time = "");

	void OrderAction(CQdpFtdcOrderActionField *pOrderAction);

	void InitTestpOrderAction(string ExchangeID);

	//撤单错误响应
	void OnRspOrderAction(CQdpFtdcOrderActionField *pInputOrderAction, CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//撤单错误
	void OnErrRtnOrderAction(CQdpFtdcOrderActionField *pOrderAction, CQdpFtdcRspInfoField *pRspInfo);
	
	///用户口令更新请求响应
    void OnRspUserPasswordUpdate(CQdpFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                 CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询行情响应
    void OnRspQryDepthMarketData(CQdpFtdcDepthMarketDataField *pDepthMarketData,
                                 CQdpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    //登陆是否报错
    bool IsErrorRspInfo(CQdpFtdcRspInfoField *pRspInfo);

	//得到BrokerID
	string getBrokerID();
	char *getCharBrokerID();

	//得到UserID
	string getUserID();
	char *getCharUserID();

	//得到Password
	string getPassword();
	char *getCharPassword();

	//得到requestID
	int getRequestID();

	//得到交易日期
	char *getCharTradingDate();

	//释放api
	void Release();

	//设置isConfirmSettlement
	void setIsConfirmSettlement(bool isConfirmSettlement);

	//得到isConfirmSettlement
	bool getIsConfirmSettlement();

	long long getOrderRefCal();
	void setOrderRefCal(long long orderref_cal);
	long long generate_orderref();

	void setOrderInsertCount(int order_insert_count);
	int getOrderInsertCount();
	void autoOrderInsertCount();

	void setIsNeedOrderAction(bool isNeedOrderAction);
	bool getIsNeedOrderAction();

	void setIsNeedOrderInsert(bool isNeedOrderInsert);
	bool getIsNeedOrderInsert();

	void setIsNeedErrorOrderInsert(bool isNeedErrorOrderInsert);
	bool getIsNeedErrorOrderInsert();

	string getActionOrderRef();
	void setActionOrderRef(string actionOrderRef);

	string getActionOrderSysID();
	void setActionOrderSysID(string actionOrderSysID);

	ofstream *getOutFile();

	CQdpFtdcInputOrderField * getTestpInputOrder();

	CQdpFtdcOrderActionField *getTestpOrderAction();

private:
    CQdpFtdcTraderApi *tdapi;
    CQdpFtdcReqUserLoginField *loginField;
	bool isLogged;
	bool isFirstTimeLogged;
	bool isConfirmSettlement;

	bool isNeedOrderAction;
	bool isNeedOrderInsert;
	bool isNeedErrorOrderInsert;
	string actionOrderRef;
	string actionOrderSysID;

	int loginRequestID;
	int order_insert_count;
	string BrokerID;
	string UserID;
	string Password;
	long long orderref_cal;
	char * c_BrokerID;
	char * c_UserID;
	char * c_Password;
	char * c_TradingDay;
	sem_t connect_sem;
	sem_t login_sem;
	sem_t logout_sem;
	sem_t sem_ReqQrySettlementInfoConfirm;
	sem_t sem_ReqQrySettlementInfo;
	sem_t sem_ReqSettlementInfoConfirm;
	ofstream outfile;
	CQdpFtdcInputOrderField *test_pInputOrder;
	CQdpFtdcOrderActionField *test_pOrderAction;
};

#endif //QUANT_CTP_TRADE_TDSPI_H
