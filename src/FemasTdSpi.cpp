#include <iostream>
#include <cstring>
#include <thread>
#include <ratio>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <string>  // string
#include <unistd.h>


#include "FemasTdSpi.h"
#include "Utils.h"
using namespace std;
using std::string;
using namespace std::chrono;

#define DEBUG
#ifdef DEBUG
#define USER_PRINT(x) {time_t tt = system_clock::to_time_t(system_clock::now()); std::string nowt(std::ctime(&tt)); std::cerr << "[DEBUG] - " << nowt.substr(0, nowt.length() - 1) << ", " << __FILE__ << ", Line - " << __LINE__ << " - " << #x << " = " << x << std::endl;}
#else
#define USER_PRINT(x)
#endif

//实盘账号
//const string USER_ID = "83601689";
//const string PASS = "270338";
//const string BROKER = "9999";

char error_content_femas[1024] = { 0 };

// 增加毫秒
void FemasTdSpi::timeraddMS(struct timeval *now_time, int ms) {
	now_time->tv_usec += ms * 1000;
	if (now_time->tv_usec >= 1000000) {
		now_time->tv_sec += now_time->tv_usec / 1000000;
		now_time->tv_usec %= 1000000;
	}
}

// 构造函数
FemasTdSpi::FemasTdSpi(CUstpFtdcTraderApi *tdapi) {
	/*sem_init(&connect_sem, 0, 1);
	sem_init(&login_sem, 0, 1);
	sem_init(&logout_sem, 0, 1);
	sem_init(&sem_ReqQrySettlementInfoConfirm, 0, 1);
	sem_init(&sem_ReqQrySettlementInfo, 0, 1);
	sem_init(&sem_ReqSettlementInfoConfirm, 0, 1);*/
	this->isLogged = false;
	this->isFirstTimeLogged = true;
	this->loginRequestID = 10;
    this->tdapi = tdapi;
	this->isConfirmSettlement = false;
	//this->orderref_cal = 100000300000;
	this->order_insert_count = 0;
	this->actionOrderRef = "";
	this->actionOrderSysID = "";
	this->isNeedOrderInsert = false;
	this->isNeedOrderAction = false;
	this->isNeedErrorOrderInsert = false;
	this->test_pInputOrder = new CUstpFtdcInputOrderField();
	this->test_pOrderAction = new CUstpFtdcOrderActionField();
	// 飞马日志记录
	outfile.open(Utils::getNowFileTimeName() + "_Femas.txt", ios::app);
}

//建立连接
void FemasTdSpi::Connect(char *frontAddress) {
	this->tdapi->RegisterFront(frontAddress); //24H
	//注册事件处理对象
	this->tdapi->RegisterSpi(this);
	//订阅共有流和私有流
	this->tdapi->SubscribePublicTopic(USTP_TERT_QUICK);
	this->tdapi->SubscribePrivateTopic(USTP_TERT_QUICK);
	this->tdapi->Init();
}

//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void FemasTdSpi::OnFrontConnected() {
	USER_PRINT("FemasTdSpi::OnFrontConnected")
	if (this->isFirstTimeLogged) {
		//sem_post(&connect_sem);
		//this->Login("0187", "86001525", "206029");
	}
	else {
		//sem_init(&login_sem, 0, 1);
		
		//this->Login(this->c_BrokerID, this->c_UserID, this->c_Password);
	}
}

//登录
void FemasTdSpi::Login(char *arg_BrokerID, char *arg_UserID, char *arg_Password) {
	
	this->BrokerID = arg_BrokerID;
	this->UserID = arg_UserID;
	this->Password = arg_Password;

	

	loginField = new CUstpFtdcReqUserLoginField();
	memset(loginField, 0x00, sizeof(CUstpFtdcReqUserLoginField));
	strcpy(loginField->BrokerID, arg_BrokerID);
	strcpy(loginField->UserID, arg_UserID);
	strcpy(loginField->Password, arg_Password);

	/*outfile << "FemasTdSpi::Login() loginField->BroderID = " << loginField->BrokerID
		<< ", loginField->UserID = " << loginField->UserID 
		<< ", loginField->Password = " << loginField->Password
		<< std::endl;*/

	tdapi->ReqUserLogin(loginField, this->loginRequestID);

	delete loginField;
}

///登录请求响应
void FemasTdSpi::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin,
                           CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	USER_PRINT("FemasTdSpi::OnRspUserLogin")
	USER_PRINT(bIsLast)
	if (bIsLast && !(this->IsErrorRspInfo(pRspInfo))) {
		this->isLogged = true;
		//sem_post(&login_sem);
		cout << "=================================================================================" << endl;
		cout << "||TdAPI 交易日:" << this->tdapi->GetTradingDay() << ", ";
		///交易日
		//cout << "CUstpFtdcRspUserLoginField 交易日:" << pRspUserLogin->TradingDay << ", ";
		///登录成功时间
		cout << "登录成功时间:" << pRspUserLogin->LoginTime << ", ";
		///经纪公司代码
		cout << "经纪公司代码:" << pRspUserLogin->BrokerID << ", ";
		///用户代码
		cout << "用户代码:" << pRspUserLogin->UserID << endl;
		cout << "=================================================================================" << endl;
	}
}





//查询交易所
void FemasTdSpi::QryExchange() {
	USER_PRINT("FemasTdSpi::QryExchange");
	CUstpFtdcQryExchangeField *pQryExchange = new CUstpFtdcQryExchangeField();
	strcpy(pQryExchange->ExchangeID, "");
	this->tdapi->ReqQryExchange(pQryExchange, this->loginRequestID);
	delete pQryExchange;
}

//响应查询交易所
void FemasTdSpi::OnRspQryExchange(CUstpFtdcRspExchangeField *pExchange, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("FemasTdSpi::OnRspQryExchange")
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		///交易所代码
		cout << "交易所代码" << pExchange->ExchangeID << endl;
		///交易所名称
		cout << "交易所名称" << pExchange->ExchangeName << endl;
	}
}

//查询合约
void FemasTdSpi::QryInstrument(string exchangeid, string instrumentid) {
	USER_PRINT("FemasTdSpi::QryInstrument Begin")
	CUstpFtdcQryInstrumentField *pQryInstrument = new CUstpFtdcQryInstrumentField();
	strcpy(pQryInstrument->ExchangeID, exchangeid.c_str());
	strcpy(pQryInstrument->InstrumentID, instrumentid.c_str());
	sleep(1);
	this->tdapi->ReqQryInstrument(pQryInstrument, this->loginRequestID + 1);
	USER_PRINT("FemasTdSpi::QryInstrument End")
	delete pQryInstrument;
}

//响应查询合约
void FemasTdSpi::OnRspQryInstrument(CUstpFtdcRspInstrumentField *pInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	//USER_PRINT("FemasTdSpi::OnRspQryInstrument")
	cout << "isLast" << bIsLast << endl;
	if ((!this->IsErrorRspInfo(pRspInfo))) {
		///合约代码
		cout << "合约代码:" << pInstrument->InstrumentID << ", ";
		///交易所代码
		cout << "交易所代码:" << pInstrument->ExchangeID << ", ";
		///合约名称
		cout << "合约名称:" << pInstrument->InstrumentName << ", ";
		///产品代码
		cout << "产品代码:" << pInstrument->ProductID << endl;
		/*///交割年份
		cout << "交割年份" << pInstrument->DeliveryYear << endl;
		///交割月
		cout << "交割月" << pInstrument->DeliveryMonth << endl;
		///市价单最大下单量
		cout << "市价单最大下单量" << pInstrument->MaxMarketOrderVolume << endl;
		///市价单最小下单量
		cout << "市价单最小下单量" << pInstrument->MinMarketOrderVolume << endl;
		///限价单最大下单量
		cout << "限价单最大下单量" << pInstrument->MaxLimitOrderVolume << endl;
		///限价单最小下单量
		cout << "限价单最小下单量" << pInstrument->MinLimitOrderVolume << endl;
		///合约数量乘数
		cout << "合约数量乘数" << pInstrument->VolumeMultiple << endl;
		///最小变动价位
		cout << "最小变动价位" << pInstrument->PriceTick << endl;
		///创建日
		cout << "创建日" << pInstrument->CreateDate << endl;
		///上市日
		cout << "上市日" << pInstrument->OpenDate << endl;
		///到期日
		cout << "到期日" << pInstrument->ExpireDate << endl;
		///开始交割日
		cout << "开始交割日" << pInstrument->StartDelivDate << endl;
		///结束交割日
		cout << "结束交割日" << pInstrument->EndDelivDate << endl;
		///合约生命周期状态
		cout << "合约生命周期状态" << pInstrument->InstLifePhase << endl;
		///当前是否交易
		cout << "当前是否交易" << pInstrument->IsTrading << endl;
		///持仓类型
		cout << "持仓类型" << pInstrument->PositionType << endl;
		///持仓日期类型
		cout << "持仓日期类型" << pInstrument->PositionDateType << endl;
		///多头保证金率
		cout << "多头保证金率" << pInstrument->LongMarginRatio << endl;
		///空头保证金率
		cout << "空头保证金率" << pInstrument->ShortMarginRatio << endl;
		///是否使用大额单边保证金算法
		cout << "是否使用大额单边保证金算法" << pInstrument->MaxMarginSideAlgorithm << endl;
		///基础商品代码
		cout << "基础商品代码" << pInstrument->UnderlyingInstrID << endl;
		///执行价
		cout << "执行价" << pInstrument->StrikePrice << endl;
		///期权类型
		cout << "期权类型" << pInstrument->OptionsType << endl;
		///合约基础商品乘数
		cout << "合约基础商品乘数" << pInstrument->UnderlyingMultiple << endl;
		///组合类型
		cout << "组合类型" << pInstrument->CombinationType << endl;*/
	}
}

///合约交易状态通知
void FemasTdSpi::OnRtnInstrumentStatus(CUstpFtdcInstrumentStatusField *pInstrumentStatus) {
	USER_PRINT("FemasTdSpi::OnRtnInstrumentStatus");
	if (pInstrumentStatus) {
		///交易所代码
		cout << "交易所代码" << pInstrumentStatus->ExchangeID << endl;
		///合约代码
		cout << "合约代码" << pInstrumentStatus->InstrumentID << endl;
		///合约交易状态
		cout << "合约交易状态" << pInstrumentStatus->InstrumentStatus << endl;
	}
}

//查询报单
void FemasTdSpi::QryOrder() {
	USER_PRINT("FemasTdSpi::QryOrder")
	CUstpFtdcQryOrderField *pQryOrder = new CUstpFtdcQryOrderField();
	//strcpy(pQryOrder->BrokerID, const_cast<char *>(this->getBrokerID().c_str()));
	//strcpy(pQryOrder->InvestorID, const_cast<char *>(this->getUserID().c_str()));
	int error_no = this->tdapi->ReqQryOrder(pQryOrder, this->getRequestID());
	cout << "error_no = " << error_no << endl;
	delete pQryOrder;
}

//响应查询报单;
void FemasTdSpi::OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("FemasTdSpi::OnRspQryOrder")
	if (!pOrder) {
		cout << "None Order Return!" << endl;
	}
	if (!this->IsErrorRspInfo(pRspInfo)) {
		if (pOrder) {
			cout << "=================================================================================" << endl;
			///经纪公司代码
			cout << "||经纪公司代码:" << pOrder->BrokerID << ", ";
			///投资者代码
			cout << "投资者代码:" << pOrder->InvestorID << ", ";
			///合约代码
			cout << "合约代码:" << pOrder->InstrumentID << ", ";
			///报单引用
			cout << "报单引用:" << pOrder->UserOrderLocalID << ", ";
			///用户代码
			cout << "用户代码:" << pOrder->UserID << endl;
			///报单价格条件
			cout << "||报单价格条件:" << pOrder->OrderPriceType << ", ";
			///买卖方向
			cout << "买卖方向:" << pOrder->Direction << ", ";
			///组合开平标志
			cout << "组合开平标志:" << pOrder->OffsetFlag << ", ";
			///组合投机套保标志
			cout << "组合投机套保标志:" << pOrder->HedgeFlag << ", ";
			///价格
			cout << "价格:" << pOrder->LimitPrice << endl;
			///数量
			cout << "||数量:" << pOrder->Volume << ", ";
			///有效期类型
			cout << "有效期类型:" << pOrder->TimeCondition << ", ";
			///GTD日期
			//cout << "GTD日期:" << pOrder->GTDDate << ", ";
			///成交量类型
			cout << "成交量类型:" << pOrder->VolumeCondition << ", ";
			///最小成交量
			cout << "最小成交量:" << pOrder->MinVolume << endl;

			///止损价
			cout << "止损价:" << pOrder->StopPrice << ", ";
			///强平原因
			cout << "强平原因:" << pOrder->ForceCloseReason << ", ";
			///自动挂起标志
			cout << "自动挂起标志:" << pOrder->IsAutoSuspend << ", ";
			///业务单元
			cout << "业务单元:" << pOrder->BusinessUnit << endl;
			cout << "=================================================================================" << endl;
		} else {
			cout << "none order return!" << endl;
		}
		
	}

}



//查询投资者
void FemasTdSpi::QryInvestor() {
	CUstpFtdcQryInvestorAccountField *pQryInvestor = new CUstpFtdcQryInvestorAccountField();
	this->tdapi->ReqQryInvestorAccount(pQryInvestor, this->getRequestID());
	delete pQryInvestor;
}

//查询投资者响应
void FemasTdSpi::OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pInvestor, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("FemasTdSpi::OnRspQryInvestorAccount");
	if (!this->IsErrorRspInfo(pRspInfo)) {
		if (pInvestor) {
			cout << "=================================================================================" << endl;
			///投资者代码
			cout << "||投资者代码:" << pInvestor->InvestorID << ", ";
			///经纪公司代码
			cout << "经纪公司代码:" << pInvestor->BrokerID << ", ";
			cout << "=================================================================================" << endl;
		}
	}
}



//查询成交单
void FemasTdSpi::QryTrade() {
	USER_PRINT("FemasTdSpi::QryTrade");
	CUstpFtdcQryTradeField *pQryTrade = new CUstpFtdcQryTradeField();
	this->tdapi->ReqQryTrade(pQryTrade, this->getRequestID());
	delete pQryTrade;
}

//查询成交单响应
void FemasTdSpi::OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	USER_PRINT("FemasTdSpi::OnRspQryTrade");
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (pTrade) {
			cout << "=================================================================================" << endl;
			///经纪公司代码
			cout << "||经纪公司代码:" << pTrade->BrokerID << ",";
			///投资者代码
			cout << "投资者代码:" << pTrade->InvestorID << ",";
			///合约代码
			cout << "合约代码:" << pTrade->InstrumentID << ",";
			///用户代码
			cout << "用户代码:" << pTrade->UserID << endl;
			///交易所代码
			cout << "||交易所代码:" << pTrade->ExchangeID << ",";
			///成交编号
			cout << "成交编号:" << pTrade->TradeID << ",";
			///买卖方向
			cout << "买卖方向:" << pTrade->Direction << ",";
			///报单编号
			cout << "报单编号:" << pTrade->OrderSysID << ",";
			///会员代码
			cout << "会员代码:" << pTrade->ParticipantID << endl;
			///客户代码
			cout << "||客户代码:" << pTrade->ClientID << ",";
			
			///开平标志
			cout << "开平标志:" << pTrade->OffsetFlag << ",";
			///投机套保标志
			cout << "投机套保标志:" << pTrade->HedgeFlag << endl;
			cout << "=================================================================================" << endl;
		}
	}
}

//下单
void FemasTdSpi::OrderInsert(char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef, string insert_time) {
	CUstpFtdcInputOrderField *pInputOrder = new CUstpFtdcInputOrderField();
	memset(pInputOrder, 0, sizeof(CUstpFtdcInputOrderField));
	/*cout << "***************************" << endl;
	cout << "broker id = " << this->getBrokerID().c_str() << endl;
	cout << "user id = " << this->getUserID().c_str() << endl;
	cout << "instrument id = " << InstrumentID << endl;
	cout << "Order Ref = " << OrderRef << endl;
	cout << "Direction = " << USTP_FTDC_D_Buy << endl;
	cout << "CombOffsetFlag = " << CombOffsetFlag << endl;
	cout << "Price = " << Price << endl;
	cout << "Volume = " << Volume << endl;
	cout << "Request id = " << this->getRequestID() << endl;*/


	///经纪公司代码
	strcpy(pInputOrder->BrokerID, this->getBrokerID().c_str());

	///投资者代码
	strcpy(pInputOrder->UserID, this->getUserID().c_str());
	//strcpy(pInputOrder->InvestorID, "801859");
	strcpy(pInputOrder->InvestorID, "801867");

	///交易所代码
	strcpy(pInputOrder->ExchangeID, "SHFE");

	///合约代码
	//string heyue = "cu1609";
	//strcpy(pInputOrder->InstrumentID, "cu1609");
	//cout << "instrument ID c_str()" << InstrumentID.c_str() << endl;

	std::strcpy(pInputOrder->InstrumentID, InstrumentID);

	//memcpy(pInputOrder->InstrumentID, InstrumentID.c_str(), InstrumentID.size() + 1);

	///报单引用
	strcpy(pInputOrder->UserOrderLocalID, OrderRef.c_str());

	///用户代码
	//strcpy(pInputOrder->UserID, this->getUserID().c_str());

	///报单价格条件
	//TThostFtdcOrderPriceTypeType	OrderPriceType; //char 任意价 '1'
	pInputOrder->OrderPriceType = USTP_FTDC_OPT_LimitPrice;

	///买卖方向
	//TThostFtdcDirectionType	Direction; //char 0买1卖
	pInputOrder->Direction = Direction;

	///组合开平标志
	//TThostFtdcCombOffsetFlagType	CombOffsetFlag; //char s[5]
	//strcpy(pInputOrder->CombOffsetFlag, CombOffsetFlag); //组合开平标志 开0平1强平2平今3平昨4
	pInputOrder->OffsetFlag = CombOffsetFlag;

	///组合投机套保标志
	//TThostFtdcCombHedgeFlagType	CombHedgeFlag; //char s[5]
	//strcpy(pInputOrder->CombHedgeFlag, "1"); //"1"投机, "2"套利, "3"套保
	pInputOrder->HedgeFlag = '1';

	///价格
	//TThostFtdcPriceType	LimitPrice; //double
	pInputOrder->LimitPrice = Price;

	///数量
	//TThostFtdcVolumeType	VolumeTotalOriginal; //int
	pInputOrder->Volume = Volume;

	///有效期类型
	//TThostFtdcTimeConditionType	TimeCondition; //char 当日有效：'3'
	pInputOrder->TimeCondition = USTP_FTDC_TC_GFD; //当日有效

	///GTD日期
	//TThostFtdcDateType	GTDDate; //char s[9]
	///成交量类型
	//TThostFtdcVolumeConditionType	VolumeCondition; //char 任何数量 '1'
	pInputOrder->VolumeCondition = USTP_FTDC_VC_AV; //任何数量 '1'

	///最小成交量
	//TThostFtdcVolumeType	MinVolume; //int
	pInputOrder->MinVolume = 1;

	///触发条件
	//TThostFtdcContingentConditionType	ContingentCondition; //char 立即 '1'
	//pInputOrder->ContingentCondition = USTP_FTDC_CC_Immediately;
	

	///止损价
	//TThostFtdcPriceType	StopPrice; //double 置为0
	//pInputOrder->StopPrice = 0;

	///强平原因
	//TThostFtdcForceCloseReasonType	ForceCloseReason; //char 非强平 '0'
	pInputOrder->ForceCloseReason = USTP_FTDC_FCR_NotForceClose;

	///自动挂起标志
	//TThostFtdcBoolType	IsAutoSuspend; //bool false
	pInputOrder->IsAutoSuspend = 0;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit; // char s[21]

	///请求编号
	//TThostFtdcRequestIDType	RequestID; //int
	//pInputOrder->RequestID = this->getRequestID();

	///用户强评标志
	//TThostFtdcBoolType	UserForceClose; //int
	//pInputOrder->UserForceClose = 0;
	

	///互换单标志
	//TThostFtdcBoolType	IsSwapOrder; //bool false
	///交易所代码
	//TThostFtdcExchangeIDType	ExchangeID;
	///投资单元代码
	//TThostFtdcInvestUnitIDType	InvestUnitID;
	///资金账号
	//TThostFtdcAccountIDType	AccountID;
	///币种代码
	//TThostFtdcCurrencyIDType	CurrencyID;
	///交易编码
	//TThostFtdcClientIDType	ClientID;
	///IP地址
	//TThostFtdcIPAddressType	IPAddress;
	///Mac地址
	//TThostFtdcMacAddressType	MacAddress;

	//outfile << "FemasTdSpi::OrderInsert() 【FemasTimeCal = " << Utils::getNowTimeNs() << std::endl;
	outfile << "FemasTdSpi::OrderInsert()\t【FemasTimeCal = " << pInputOrder->UserOrderLocalID << ", " << pInputOrder->OrderSysID << ", " << Utils::getNowTimeNs() << ", ";
	this->isNeedOrderInsert = false;
	this->isNeedErrorOrderInsert = false;
	this->tdapi->ReqOrderInsert(pInputOrder, this->getRequestID());
	
	delete pInputOrder;
}

void FemasTdSpi::OrderInsert(CUstpFtdcInputOrderField *pInputOrder) {
	///报单引用
	strcpy(pInputOrder->UserOrderLocalID, Utils::longlongtostr(this->generate_orderref()).c_str());
	//outfile << "FemasTdSpi::OrderInsert()\t【FemasTimeCal = " << pInputOrder->UserOrderLocalID << ", " << pInputOrder->OrderSysID << ", " << Utils::getNowTimeNs() << ", ";
	outfile << pInputOrder->UserOrderLocalID << ", " << pInputOrder->OrderSysID << ", " << Utils::getNowTimeNs() << ", ";
	this->isNeedOrderInsert = false;
	this->isNeedErrorOrderInsert = false;
	this->tdapi->ReqOrderInsert(pInputOrder, this->getRequestID());
}

void FemasTdSpi::InitTestpInputOrder(char *InstrumentID, char CombOffsetFlag, char Direction, int Volume, double Price, string OrderRef, string insert_time) {

	memset(this->test_pInputOrder, 0, sizeof(CUstpFtdcInputOrderField));
	/*cout << "***************************" << endl;
	cout << "broker id = " << this->getBrokerID().c_str() << endl;
	cout << "user id = " << this->getUserID().c_str() << endl;
	cout << "instrument id = " << InstrumentID << endl;
	cout << "Order Ref = " << OrderRef << endl;
	cout << "Direction = " << USTP_FTDC_D_Buy << endl;
	cout << "CombOffsetFlag = " << CombOffsetFlag << endl;
	cout << "Price = " << Price << endl;
	cout << "Volume = " << Volume << endl;
	cout << "Request id = " << this->getRequestID() << endl;*/

	/*错单测试需要，注释BroderID, InvestorID*/

	///经纪公司代码
	strcpy(this->test_pInputOrder->BrokerID, this->getBrokerID().c_str());

	///投资者代码
	strcpy(this->test_pInputOrder->UserID, this->getUserID().c_str());

	strcpy(this->test_pInputOrder->InvestorID, "801867");

	///交易所代码
	strcpy(this->test_pInputOrder->ExchangeID, "SHFE");

	///合约代码
	//string heyue = "cu1609";
	//strcpy(this->test_pInputOrder->InstrumentID, "cu1609");
	//cout << "instrument ID c_str()" << InstrumentID.c_str() << endl;

	std::strcpy(this->test_pInputOrder->InstrumentID, InstrumentID);

	//memcpy(this->test_pInputOrder->InstrumentID, InstrumentID.c_str(), InstrumentID.size() + 1);

	///报单引用
	//strcpy(this->test_pInputOrder->UserOrderLocalID, OrderRef.c_str());

	///用户代码
	//strcpy(this->test_pInputOrder->UserID, this->getUserID().c_str());

	///报单价格条件
	//TThostFtdcOrderPriceTypeType	OrderPriceType; //char 任意价 '1'
	this->test_pInputOrder->OrderPriceType = USTP_FTDC_OPT_LimitPrice;

	///买卖方向
	//TThostFtdcDirectionType	Direction; //char 0买1卖
	this->test_pInputOrder->Direction = Direction;

	///组合开平标志
	//TThostFtdcCombOffsetFlagType	CombOffsetFlag; //char s[5]
	//strcpy(this->test_pInputOrder->CombOffsetFlag, CombOffsetFlag); //组合开平标志 开0平1强平2平今3平昨4
	this->test_pInputOrder->OffsetFlag = CombOffsetFlag;

	///组合投机套保标志
	//TThostFtdcCombHedgeFlagType	CombHedgeFlag; //char s[5]
	//strcpy(this->test_pInputOrder->CombHedgeFlag, "1"); //"1"投机, "2"套利, "3"套保
	this->test_pInputOrder->HedgeFlag = '1';

	///价格
	//TThostFtdcPriceType	LimitPrice; //double
	this->test_pInputOrder->LimitPrice = Price;

	///数量
	//TThostFtdcVolumeType	VolumeTotalOriginal; //int
	this->test_pInputOrder->Volume = Volume;

	///有效期类型
	//TThostFtdcTimeConditionType	TimeCondition; //char 当日有效：'3'
	this->test_pInputOrder->TimeCondition = USTP_FTDC_TC_GFD; //当日有效

	///GTD日期
	//TThostFtdcDateType	GTDDate; //char s[9]
	///成交量类型
	//TThostFtdcVolumeConditionType	VolumeCondition; //char 任何数量 '1'
	this->test_pInputOrder->VolumeCondition = USTP_FTDC_VC_AV; //任何数量 '1'

	///最小成交量
	//TThostFtdcVolumeType	MinVolume; //int
	this->test_pInputOrder->MinVolume = 1;

	///触发条件
	//TThostFtdcContingentConditionType	ContingentCondition; //char 立即 '1'
	//this->test_pInputOrder->ContingentCondition = USTP_FTDC_CC_Immediately;


	///止损价
	//TThostFtdcPriceType	StopPrice; //double 置为0
	//this->test_pInputOrder->StopPrice = 0;

	///强平原因
	//TThostFtdcForceCloseReasonType	ForceCloseReason; //char 非强平 '0'
	this->test_pInputOrder->ForceCloseReason = USTP_FTDC_FCR_NotForceClose;

	///自动挂起标志
	//TThostFtdcBoolType	IsAutoSuspend; //bool false
	this->test_pInputOrder->IsAutoSuspend = 0;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit; // char s[21]

	///请求编号
	//TThostFtdcRequestIDType	RequestID; //int
	//this->test_pInputOrder->RequestID = this->getRequestID();

	///用户强评标志
	//TThostFtdcBoolType	UserForceClose; //int
	//this->test_pInputOrder->UserForceClose = 0;


	///互换单标志
	//TThostFtdcBoolType	IsSwapOrder; //bool false
	///交易所代码
	//TThostFtdcExchangeIDType	ExchangeID;
	///投资单元代码
	//TThostFtdcInvestUnitIDType	InvestUnitID;
	///资金账号
	//TThostFtdcAccountIDType	AccountID;
	///币种代码
	//TThostFtdcCurrencyIDType	CurrencyID;
	///交易编码
	//TThostFtdcClientIDType	ClientID;
	///IP地址
	//TThostFtdcIPAddressType	IPAddress;
	///Mac地址
	//TThostFtdcMacAddressType	MacAddress;

	//outfile << "FemasTdSpi::OrderInsert() 【FemasTimeCal = " << Utils::getNowTimeNs() << std::endl;
}

///报单录入请求响应
void FemasTdSpi::OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (pInputOrder) {
			/*cout << "=================================================================================" << endl;
			///经纪公司编号
			cout << "经纪公司编号:" << pInputOrder->BrokerID << ", ";
			///交易所代码
			cout << "交易所代码:" << pInputOrder->ExchangeID << ", ";
			///系统报单编号
			cout << "系统报单编号:" << pInputOrder->OrderSysID << ", ";
			///投资者编号
			cout << "投资者编号:" << pInputOrder->InvestorID << ", ";
			///用户代码
			cout << "用户代码:" << pInputOrder->UserID << ", ";
			///合约代码
			cout << "合约代码:" << pInputOrder->InstrumentID << ", ";
			///用户本地报单号
			cout << "用户本地报单号:" << pInputOrder->UserOrderLocalID << ", ";
			///报单类型
			cout << "报单类型:" << pInputOrder->OrderPriceType << ", ";
			///买卖方向
			cout << "买卖方向:" << pInputOrder->Direction << ", ";
			///开平标志
			cout << "开平标志:" << pInputOrder->OffsetFlag << ", ";
			///投机套保标志
			cout << "投机套保标志:" << pInputOrder->HedgeFlag << ", ";
			///价格
			cout << "价格:" << pInputOrder->LimitPrice << ", ";
			///数量
			cout << "数量:" << pInputOrder->Volume << ", ";
			///有效期类型
			cout << "有效期类型:" << pInputOrder->TimeCondition << ", ";
			///GTD日期
			cout << "GTD日期:" << pInputOrder->GTDDate << ", ";
			///成交量类型
			cout << "成交量类型:" << pInputOrder->VolumeCondition << ", ";
			///最小成交量
			cout << "最小成交量:" << pInputOrder->MinVolume << ", ";
			///止损价
			cout << "止损价:" << pInputOrder->StopPrice << ", ";
			///强平原因
			cout << "强平原因:" << pInputOrder->ForceCloseReason << ", ";
			///自动挂起标志
			cout << "自动挂起标志:" << pInputOrder->IsAutoSuspend << ", ";
			///业务单元
			cout << "业务单元:" << pInputOrder->BusinessUnit << ", ";
			///用户自定义域
			cout << "用户自定义域:" << pInputOrder->UserCustom << ", ";
			///本地业务标识
			cout << "本地业务标识:" << pInputOrder->BusinessLocalID << ", ";
			///业务发生日期
			cout << "业务发生日期:" << pInputOrder->ActionDay << endl;
			cout << "=================================================================================" << endl;*/

			outfile << Utils::getNowTimeNs() << ", ";
			this->isNeedOrderAction = false;
			this->isNeedOrderInsert = false;
		}
	}
}

//下单响应
void FemasTdSpi::OnRtnOrder(CUstpFtdcOrderField *pOrder) {
	
	if (pOrder) {
		/*cout << "=================================================================================" << endl;
		///经纪公司编号
		cout << "经纪公司编号:" << pOrder->BrokerID << ", ";
		///交易所代码
		cout << "交易所代码:" << pOrder->ExchangeID << ", ";
		///系统报单编号
		cout << "系统报单编号:" << pOrder->OrderSysID << ", ";
		///投资者编号
		cout << "投资者编号:" << pOrder->InvestorID << ", ";
		///用户代码
		cout << "用户代码:" << pOrder->UserID << ", ";
		///合约代码
		cout << "合约代码:" << pOrder->InstrumentID << ", ";
		///用户本地报单号
		cout << "用户本地报单号:" << pOrder->UserOrderLocalID << ", ";
		///报单类型
		cout << "报单类型:" << pOrder->OrderPriceType << ", ";
		///买卖方向
		cout << "买卖方向:" << pOrder->Direction << ", ";
		///开平标志
		cout << "开平标志:" << pOrder->OffsetFlag << ", ";
		///投机套保标志
		cout << "投机套保标志:" << pOrder->HedgeFlag << ", ";
		///价格
		cout << "价格:" << pOrder->LimitPrice << ", ";
		///数量
		cout << "数量:" << pOrder->Volume << ", ";
		///有效期类型
		cout << "有效期类型:" << pOrder->TimeCondition << ", ";
		///GTD日期
		cout << "GTD日期:" << pOrder->GTDDate << ", ";
		///成交量类型
		cout << "成交量类型:" << pOrder->VolumeCondition << ", ";
		///最小成交量
		cout << "最小成交量:" << pOrder->MinVolume << ", ";
		///止损价
		cout << "止损价:" << pOrder->StopPrice << ", ";
		///强平原因
		cout << "强平原因:" << pOrder->ForceCloseReason << ", ";
		///自动挂起标志
		cout << "自动挂起标志:" << pOrder->IsAutoSuspend << ", ";
		///业务单元
		cout << "业务单元:" << pOrder->BusinessUnit << ", ";
		///用户自定义域
		cout << "用户自定义域:" << pOrder->UserCustom << ", ";
		///本地业务标识
		cout << "本地业务标识:" << pOrder->BusinessLocalID << ", ";
		///业务发生日期
		cout << "业务发生日期:" << pOrder->ActionDay << ", ";
		///交易日
		cout << "交易日:" << pOrder->TradingDay << ", ";
		///会员编号
		cout << "会员编号:" << pOrder->ParticipantID << ", ";
		///客户号
		cout << "客户号:" << pOrder->ClientID << ", ";
		///下单席位号
		cout << "下单席位号:" << pOrder->SeatID << ", ";
		///插入时间
		cout << "插入时间:" << pOrder->InsertTime << ", ";
		///本地报单编号
		cout << "本地报单编号:" << pOrder->OrderLocalID << ", ";
		///报单来源
		cout << "报单来源:" << pOrder->OrderSource << ", ";
		///报单状态
		cout << "报单状态:" << pOrder->OrderStatus << ", ";
		///撤销时间
		cout << "撤销时间:" << pOrder->CancelTime << ", ";
		///撤单用户编号
		cout << "撤单用户编号:" << pOrder->CancelUserID << ", ";
		///今成交数量
		cout << "今成交数量:" << pOrder->VolumeTraded << ", ";
		///剩余数量
		cout << "剩余数量:" << pOrder->VolumeRemain << endl;
		cout << "=================================================================================" << endl;*/

		
		
		
		if (pOrder->OrderStatus == '3') // 挂单未成交
		{
			outfile << Utils::getNowTimeNs() << ", 报单" << std::endl;
			//sleep(1);
#if 0
			usleep(500000);
			this->OrderAction("SHFE", pOrder->UserOrderLocalID, pOrder->OrderSysID, Utils::longlongtostr(this->generate_orderref()));
#endif
			this->actionOrderRef = pOrder->UserOrderLocalID;
			this->actionOrderSysID = pOrder->OrderSysID;
			this->isNeedOrderAction = true;
			this->isNeedOrderInsert = false;

		}
		else if (pOrder->OrderStatus == '5') {
			outfile << Utils::getNowTimeNs() << ", 撤单" << std::endl;

#if 0
			usleep(500000);
			this->order_insert_count += 1;
			if (this->order_insert_count > 399)
			{
			}
			else {
				//outfile << "FemasTdSpi::OnRtnOrder() 第 " << this->order_insert_count << " 次发单" << std::endl;
				//报单
				//this->OrderInsert("rb1801", '0', '0', 1, 3457, Utils::longlongtostr(this->generate_orderref()));
				//this->OrderInsert("al1712", '0', '0', 1, 15575, Utils::longlongtostr(this->generate_orderref()));
				//this->OrderInsert("rb1805", '0', '0', 1, 3290, Utils::longlongtostr(this->generate_orderref()));
				//this->OrderInsert("rb1712", '0', '0', 1, 3555, Utils::longlongtostr(this->generate_orderref()));
				this->OrderInsert("al1801", '0', '0', 1, 15350, Utils::longlongtostr(this->generate_orderref()));
			}
#endif
			this->isNeedOrderInsert = true;
			this->isNeedOrderAction = false;

		}
		else {
			outfile << "FemasTdSpi::OnRtnOrder() 【异常!】" << std::endl;
		}
	}
}

///成交回报
void FemasTdSpi::OnRtnTrade(CUstpFtdcTradeField *pTrade) {
	if (pTrade)
	{
		cout << "=================================================================================" << endl;
		///经纪公司编号
		cout << "经纪公司编号:" << pTrade->BrokerID << ", ";
		///交易所代码
		cout << "交易所代码:" << pTrade->ExchangeID << ", ";
		///交易日
		cout << "交易日:" << pTrade->TradingDay << ", ";
		///会员编号
		cout << "会员编号:" << pTrade->ParticipantID << ", ";
		///下单席位号
		cout << "下单席位号:" << pTrade->SeatID << ", ";
		///投资者编号
		cout << "投资者编号:" << pTrade->InvestorID << ", ";
		///客户号
		cout << "客户号:" << pTrade->ClientID << ", ";
		///用户编号
		cout << "用户编号:" << pTrade->UserID << ", ";
		///成交编号
		cout << "成交编号:" << pTrade->TradeID << ", ";
		///报单编号
		cout << "报单编号:" << pTrade->OrderSysID << ", ";
		///本地报单编号
		cout << "本地报单编号:" << pTrade->UserOrderLocalID << ", ";
		///合约代码
		cout << "合约代码:" << pTrade->InstrumentID << ", ";
		///买卖方向
		cout << "买卖方向:" << pTrade->Direction << ", ";
		///开平标志
		cout << "开平标志:" << pTrade->OffsetFlag << ", ";
		///投机套保标志
		cout << "投机套保标志:" << pTrade->HedgeFlag << ", ";
		///成交价格
		cout << "成交价格:" << pTrade->TradePrice << ", ";
		///成交数量
		cout << "成交数量:" << pTrade->TradeVolume << ", ";
		///成交时间
		cout << "成交时间:" << pTrade->TradeTime << ", ";
		///清算会员编号
		cout << "清算会员编号:" << pTrade->ClearingPartID << ", ";
		///本地业务标识
		cout << "本地业务标识:" << pTrade->BusinessLocalID << ", ";
		///业务发生日期
		cout << "业务发生日期:" << pTrade->ActionDay << endl;
		cout << "=================================================================================" << endl;

		outfile << "FemasTdSpi::OnRtnTrade() 【FemasTimeCal = " << pTrade->UserOrderLocalID << ", " << pTrade->OrderSysID << ", " << Utils::getNowTimeNs() << std::endl;
	}
}

//下单错误响应
void FemasTdSpi::OnErrRtnOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo) {
	outfile << Utils::getNowTimeNs() << ", 错单" << std::endl;
	//usleep(10000);
	//usleep(1000000);

	//this->order_insert_count += 1;
	//if (this->order_insert_count > 199)
	//{
	//	//报单
	//	//this->OrderInsert("al1801", '0', '0', 1, 15195, Utils::longlongtostr(this->generate_orderref()));
	//}
	//else {
	//	//outfile << "FemasTdSpi::OnRtnOrder() 第 " << this->order_insert_count << " 次发单" << std::endl;
	//	//报单
	//	this->OrderInsert("rb1801", '0', '0', 1, 3366, Utils::longlongtostr(this->generate_orderref()));
	//}
	
	this->isNeedErrorOrderInsert = true;
}

//撤单
void FemasTdSpi::OrderAction(string ExchangeID, string OrderRef, string OrderSysID, string UserOrderActionLocalID, string action_time) {
	CUstpFtdcOrderActionField *pOrderAction = new CUstpFtdcOrderActionField();
	strcpy(pOrderAction->BrokerID, this->getBrokerID().c_str());
	strcpy(pOrderAction->UserID, this->getUserID().c_str());
	//strcpy(pOrderAction->InvestorID, "801859");
	strcpy(pOrderAction->InvestorID, "801867");
	//strcpy(f->InstrumentID, "a1501");
	strcpy(pOrderAction->ExchangeID, ExchangeID.c_str());
	strcpy(pOrderAction->UserOrderLocalID, OrderRef.c_str());		//设置报单引用
	strcpy(pOrderAction->OrderSysID, OrderSysID.c_str());
	strcpy(pOrderAction->UserOrderActionLocalID, UserOrderActionLocalID.c_str());
	pOrderAction->ActionFlag = USTP_FTDC_AF_Delete; //删除

	/*outfile << "FemasTdSpi::OrderAction() BrokerID = " << pOrderAction->BrokerID
		<< ", UserID = " << pOrderAction->UserID
		<< ", InvestorID = " << pOrderAction->InvestorID
		<< ", ExchangeID = " << pOrderAction->ExchangeID
		<< ", UserOrderLocalID = " << pOrderAction->UserOrderLocalID
		<< ", UserOrderActionLocalID = " << pOrderAction->UserOrderActionLocalID
		<< ", OrderSysID = " << pOrderAction->OrderSysID
		<< std::endl;*/

	//outfile << "FemasTdSpi::OrderAction()\t【FemasTimeCal = " << pOrderAction->UserOrderActionLocalID << ", " << pOrderAction->OrderSysID << ", " << Utils::getNowTimeNs() << ", ";
	outfile << pOrderAction->UserOrderActionLocalID << ", " << pOrderAction->OrderSysID << ", " << Utils::getNowTimeNs() << ", ";
	this->isNeedOrderAction = false;
	this->tdapi->ReqOrderAction(pOrderAction, this->getRequestID());
	delete pOrderAction;
}

void FemasTdSpi::InitTestpOrderAction(string ExchangeID) {
	
	strcpy(this->test_pOrderAction->BrokerID, this->getBrokerID().c_str());
	strcpy(this->test_pOrderAction->UserID, this->getUserID().c_str());
	//strcpy(this->test_pOrderAction->InvestorID, "801859");
	strcpy(this->test_pOrderAction->InvestorID, "801867");
	//strcpy(f->InstrumentID, "a1501");
	strcpy(this->test_pOrderAction->ExchangeID, ExchangeID.c_str());
	
	this->test_pOrderAction->ActionFlag = USTP_FTDC_AF_Delete; //删除
}

void FemasTdSpi::OrderAction(CUstpFtdcOrderActionField *pOrderAction) {

	strcpy(pOrderAction->UserOrderLocalID, this->getActionOrderRef().c_str());	//设置撤单的本地报单编号
	strcpy(pOrderAction->OrderSysID, this->getActionOrderSysID().c_str());		//设置撤单系统编号
	strcpy(pOrderAction->UserOrderActionLocalID, Utils::longlongtostr(this->generate_orderref()).c_str());
	outfile << pOrderAction->UserOrderActionLocalID << ", " << pOrderAction->OrderSysID << ", " << Utils::getNowTimeNs() << ", ";
	this->isNeedOrderAction = false;
	this->tdapi->ReqOrderAction(pOrderAction, this->getRequestID());
}

//撤单错误响应
void FemasTdSpi::OnRspOrderAction(CUstpFtdcOrderActionField *pInputOrderAction, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!(this->IsErrorRspInfo(pRspInfo))) {
		if (pInputOrderAction)
		{
			outfile << Utils::getNowTimeNs() << ", ";
		}
	}
}

//撤单错误
void FemasTdSpi::OnErrRtnOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo) {
	outfile << "FemasTdSpi::OnErrRtnOrderAction() 【FemasTimeCal = " << Utils::getNowTimeNs() << std::endl;
	if (!(this->IsErrorRspInfo(pRspInfo))) {

	}
}



//登出
void FemasTdSpi::Logout(char *BrokerID, char *UserID) {
	CUstpFtdcReqUserLogoutField *pUserLogout = new CUstpFtdcReqUserLogoutField();
	strcpy(pUserLogout->BrokerID, BrokerID);
	strcpy(pUserLogout->UserID, UserID);
	this->tdapi->ReqUserLogout(pUserLogout, this->loginRequestID);

}

///登出请求响应
void FemasTdSpi::OnRspUserLogout(CUstpFtdcRspUserLoginField *pUserLogout,
                            CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (bIsLast && !(this->IsErrorRspInfo(pRspInfo))) {
		USER_PRINT("FemasTdSpi::OnRspUserLogout")
		sem_post(&logout_sem);
	}
}

//等待线程结束
void FemasTdSpi::Join() {
	USER_PRINT("FemasTdSpi::Join()")
	this->tdapi->Join();
}



///用户口令更新请求响应
void FemasTdSpi::OnRspUserPasswordUpdate(CUstpFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                    CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cout << "回调用户口令更新请求响应OnRspUserPasswordUpdate" << endl;
    if (pRspInfo->ErrorID == 0){
        cout << "更改成功 " << endl
        << "旧密码为:" << pUserPasswordUpdate->OldPassword << endl
        << "新密码为:" << pUserPasswordUpdate->NewPassword << endl;
    }
    else{
        cout << pRspInfo->ErrorID << ends << pRspInfo->ErrorMsg << endl;
    }
}

///请求查询行情响应
void FemasTdSpi::OnRspQryDepthMarketData(CUstpFtdcDepthMarketDataField *pDepthMarketData,
                                    CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cout << "OnRspQryDepthMarketData" << endl;
    cout << nRequestID << endl;
    if (pDepthMarketData != NULL){
        cout << "-----------------行情数据--------------------" << endl;
        cout << "交易日:" << pDepthMarketData->TradingDay << endl
        << "合约代码:" << pDepthMarketData->InstrumentID << endl
        << "最新价:" << pDepthMarketData->LastPrice << endl
        << "最高价:" << pDepthMarketData->HighestPrice << endl
        << "最低价:" << pDepthMarketData->LowestPrice << endl;
        cout << "-----------------行情数据--------------------" << endl;
    }
}

bool FemasTdSpi::IsErrorRspInfo(CUstpFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult) {
		error_content_femas[1024] = { 0 };
		Utils::g2u(pRspInfo->ErrorMsg, (size_t)strlen(pRspInfo->ErrorMsg), error_content_femas, (size_t)sizeof(error_content_femas));
		cerr << "FemasTdSpi::IsErrorRspInfo() ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << error_content_femas << endl;
	}
		
	return bResult;
}

//得到BrokerID
string FemasTdSpi::getBrokerID() {
	return this->BrokerID;
}

char * FemasTdSpi::getCharBrokerID() {

	return this->c_BrokerID;
}


//得到UserID
string FemasTdSpi::getUserID() {
	return this->UserID;
}

char * FemasTdSpi::getCharUserID() {
	return this->c_UserID;
}

//得到Password
string FemasTdSpi::getPassword() {
	return this->Password;
}

char * FemasTdSpi::getCharPassword() {
	return this->c_Password;
}

//得到requestID
int FemasTdSpi::getRequestID() {
	return this->loginRequestID;
}

//得到交易日期
char * FemasTdSpi::getCharTradingDate() {
	return (const_cast<char *> (this->tdapi->GetTradingDay()));
}

//设置isConfirmSettlement
void FemasTdSpi::setIsConfirmSettlement(bool isConfirmSettlement) {
	this->isConfirmSettlement = isConfirmSettlement;
}

//得到isConfirmSettlement
bool FemasTdSpi::getIsConfirmSettlement() {
	return this->isConfirmSettlement;
}

long long FemasTdSpi::getOrderRefCal() {
	return this->orderref_cal;
}
void FemasTdSpi::setOrderRefCal(long long orderref_cal) {
	this->orderref_cal = orderref_cal;
}

long long FemasTdSpi::generate_orderref() {
	this->orderref_cal += 1;
	return this->orderref_cal;
}

void FemasTdSpi::setOrderInsertCount(int order_insert_count) {
	this->order_insert_count = order_insert_count;
}
int FemasTdSpi::getOrderInsertCount() {
	return this->order_insert_count;
}

void FemasTdSpi::autoOrderInsertCount() {
	this->order_insert_count += 1;
}

void FemasTdSpi::setIsNeedOrderAction(bool isNeedOrderAction) {
	this->isNeedOrderAction = isNeedOrderAction;
}

bool FemasTdSpi::getIsNeedOrderAction() {
	return this->isNeedOrderAction;
}

void FemasTdSpi::setIsNeedOrderInsert(bool isNeedOrderInsert) {
	this->isNeedOrderInsert = isNeedOrderInsert;
}

bool FemasTdSpi::getIsNeedOrderInsert() {
	return this->isNeedOrderInsert;
}

void FemasTdSpi::setIsNeedErrorOrderInsert(bool isNeedErrorOrderInsert) {
	this->isNeedErrorOrderInsert = isNeedErrorOrderInsert;
}

bool FemasTdSpi::getIsNeedErrorOrderInsert() {
	return this->isNeedErrorOrderInsert;
}

string FemasTdSpi::getActionOrderRef() {
	return this->actionOrderRef;
}

void FemasTdSpi::setActionOrderRef(string actionOrderRef) {
	this->actionOrderRef = actionOrderRef;
}

string FemasTdSpi::getActionOrderSysID() {
	return this->actionOrderSysID;
}

void FemasTdSpi::setActionOrderSysID(string actionOrderSysID) {
	this->actionOrderSysID = actionOrderSysID;
}

ofstream * FemasTdSpi::getOutFile() {
	return &(this->outfile);
}

CUstpFtdcInputOrderField * FemasTdSpi::getTestpInputOrder() {
	return this->test_pInputOrder;
}

CUstpFtdcOrderActionField * FemasTdSpi::getTestpOrderAction() {
	return this->test_pOrderAction;
}


//释放api
void FemasTdSpi::Release() {
	this->tdapi->Release();
}