#include <iostream>
#include "QdpTdSpi.h"
#include "FemasTdSpi.h"
#include "Utils.h"
#include "Timer.h"

using namespace std;

using std::map;
using std::string;

FemasTdSpi *femas_tdspi = NULL;
QdpTdSpi *qdp_tdspi = NULL;
bool isStarted = false;


string Test_InstrumentID = "rb1801";
double Test_Price = 3408;
string test_start_time = "12:14:30"; // 启动测试时间
long long global_orderref_cal = 100000002600; // 报单引用值100000407000

#define TEST_COUNT_NUM	199

// 定义错误发单
//#define TEST_ERROR_INSERT

// 报单顺序调换
#define REVERSECALL

void timer_handler() {
	//printf("timer_handler()...");
	//std::cout << "timer_handler()..." << std::endl;

	string nowtime = Utils::getNowTime();
	string ymd_date = Utils::getYMDDate();
	//std::cout << "现在时间 = " << nowtime << std::endl;
	//std::cout << "现在日期 = " << ymd_date << std::endl;
	// 时间大于早上开盘时间
	if (Utils::compareTradingDaySeconds(nowtime.c_str(), (ymd_date + test_start_time).c_str())) {
		
		// 之前未启动
		if (!isStarted)
		{
			//std::cout << "时间被捕获" << std::endl;
			//femas_tdspi->OrderInsert("rb1801", '0', '0', 1, 3421, Utils::longlongtostr(femas_tdspi->generate_orderref()));

			// 偶数时间就开始发单
			if (!Utils::isOrdTime())
			{
				//string insert_time = Utils::getNowTimeNs();
				//qdp_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(qdp_tdspi->generate_orderref()), insert_time);
				//femas_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(femas_tdspi->generate_orderref()), insert_time);
				
#ifdef REVERSECALL
				qdp_tdspi->OrderInsert(qdp_tdspi->getTestpInputOrder());
				femas_tdspi->OrderInsert(femas_tdspi->getTestpInputOrder());
#else
				femas_tdspi->OrderInsert(femas_tdspi->getTestpInputOrder());
				qdp_tdspi->OrderInsert(qdp_tdspi->getTestpInputOrder());
#endif	
				isStarted = true;
			}

		}

#ifndef TEST_ERROR_INSERT
		else {

			if (femas_tdspi->getIsNeedOrderAction() && qdp_tdspi->getIsNeedOrderAction())
			{
				//奇数时间就撤单
				if (Utils::isOrdTime())
				{
					string action_time = Utils::getNowTimeNs();				
					
#ifdef REVERSECALL
					qdp_tdspi->OrderAction(qdp_tdspi->getTestpOrderAction());
					femas_tdspi->OrderAction(femas_tdspi->getTestpOrderAction());
#else
					femas_tdspi->OrderAction(femas_tdspi->getTestpOrderAction());
					qdp_tdspi->OrderAction(qdp_tdspi->getTestpOrderAction());	
#endif
				}
			}
			if (femas_tdspi->getIsNeedOrderInsert() && qdp_tdspi->getIsNeedOrderInsert())
			{
				femas_tdspi->autoOrderInsertCount();
				if (femas_tdspi->getOrderInsertCount() > TEST_COUNT_NUM)
				{
					femas_tdspi->getOutFile()->close();
					qdp_tdspi->getOutFile()->close();
				}
				else {
					//偶数时间就发单
					if (!Utils::isOrdTime())
					{
						//string insert_time = Utils::getNowTimeNs();
						
						//qdp_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(qdp_tdspi->generate_orderref()), insert_time);
						//femas_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(femas_tdspi->generate_orderref()), insert_time);

#ifdef REVERSECALL
						qdp_tdspi->OrderInsert(qdp_tdspi->getTestpInputOrder());
						femas_tdspi->OrderInsert(femas_tdspi->getTestpInputOrder());
#else
						femas_tdspi->OrderInsert(femas_tdspi->getTestpInputOrder());
						qdp_tdspi->OrderInsert(qdp_tdspi->getTestpInputOrder());
#endif	
					}
				}

			}
		}
#endif

#ifdef TEST_ERROR_INSERT
	else {
		if (femas_tdspi->getIsNeedErrorOrderInsert() && qdp_tdspi->getIsNeedErrorOrderInsert())
		{
			femas_tdspi->autoOrderInsertCount();
			if (femas_tdspi->getOrderInsertCount() > TEST_COUNT_NUM)
			{
				femas_tdspi->getOutFile()->close();
				qdp_tdspi->getOutFile()->close();
			}
			else {

				//string insert_time = Utils::getNowTimeNs();
				//qdp_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(qdp_tdspi->generate_orderref()), insert_time);
				//femas_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(femas_tdspi->generate_orderref()), insert_time);


#ifdef REVERSECALL
				qdp_tdspi->OrderInsert(qdp_tdspi->getTestpInputOrder());
				femas_tdspi->OrderInsert(femas_tdspi->getTestpInputOrder());
				
#else
				femas_tdspi->OrderInsert(femas_tdspi->getTestpInputOrder());
				qdp_tdspi->OrderInsert(qdp_tdspi->getTestpInputOrder());
#endif

					

			}

		}
	}
#endif

	}
}


int main() {


	// Femas TdSpi初始化
	string femas_frontAddr = "tcp://172.18.105.7:8002"; //盘中
	string femas_broker_id = "0187";
	string femas_user_id = "80186701";
	string femas_password = "gmqh122854";

	CUstpFtdcTraderApi *femas_tdapi = CUstpFtdcTraderApi::CreateFtdcTraderApi("./conn/femas/");
	femas_tdspi = new FemasTdSpi(femas_tdapi);
	femas_tdspi->setOrderRefCal(global_orderref_cal);
	//femas_tdspi->OrderInsert(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(femas_tdspi->generate_orderref()), insert_time);
	


	femas_tdspi->Connect(const_cast<char *>(femas_frontAddr.c_str()));
	sleep(1);
	femas_tdspi->Login(const_cast<char *>(femas_broker_id.c_str()), const_cast<char *>(femas_user_id.c_str()), const_cast<char *>(femas_password.c_str()));
	sleep(3);
	femas_tdspi->InitTestpInputOrder(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(femas_tdspi->generate_orderref()));
	femas_tdspi->InitTestpOrderAction("SHFE");


	// QDP TdSpi初始化
	string mini_frontAddr = "tcp://172.18.105.6:30005"; //盘中
	//string mini_broker_id = "0187";
	string mini_broker_id = "guomao";
	string mini_user_id = "801859";
	//string mini_password = "242169";
	string mini_password = "111111";
	CQdpFtdcTraderApi *qdp_tdapi = CQdpFtdcTraderApi::CreateFtdcTraderApi("./conn/qdp/");
	qdp_tdspi = new QdpTdSpi(qdp_tdapi);
	qdp_tdspi->setOrderRefCal(global_orderref_cal);

	qdp_tdspi->Connect(const_cast<char *>(mini_frontAddr.c_str()));
	sleep(1);
	qdp_tdspi->Login(const_cast<char *>(mini_broker_id.c_str()), const_cast<char *>(mini_user_id.c_str()), const_cast<char *>(mini_password.c_str()));
	sleep(3);
	qdp_tdspi->InitTestpInputOrder(const_cast<char *>(Test_InstrumentID.c_str()), '0', '0', 1, Test_Price, Utils::longlongtostr(qdp_tdspi->generate_orderref()));
	qdp_tdspi->InitTestpOrderAction("SHFE");

	


	// 创建定时器
	Timer tHello(timer_handler);
	// 开启定时器
	tHello.setSingleShot(false);
	// tHello.setInterval(Timer::Interval(1000 * 60 * 10));
	tHello.setInterval(Timer::Interval(500));
	//tHello.start(true);


	//sleep(2);
	//femas_tdspi->Login("9999", "", "");
	//femas_tdspi->Login("9999", "", "");

	/**
	实盘
	*/
	//连接
	//femas_tdspi->Connect("tcp://220.248.44.146:41205");
	//sleep(1);
	//登录
	//femas_tdspi->Login("0187", "86001525", "");
	//femas_tdspi->Login("0187", "801859", "");
	//sleep(1);
	//查询交易结算确认
	//femas_tdspi->QrySettlementInfoConfirm(const_cast<char *>(femas_tdspi->getBrokerID().c_str()), const_cast<char *>(femas_tdspi->getUserID().c_str()), femas_tdspi->getRequestID());
	//查询交易所
	//sleep(6);
	//femas_tdspi->QryExchange();
	//查询合约
	//sleep(1);
	//femas_tdspi->QryInstrument();

	//查询报单
	//sleep(1);
	//femas_tdspi->QryOrder();

	//sleep(1);
	//cout << "oh?" << endl;

	//查询保证金率
	//sleep(1);
	//femas_tdspi->QryInstrumentMarginRate();

	//查询手续费率
	//sleep(1);
	//femas_tdspi->QryInstrumentCommissionRate();

	//查询账户
	//sleep(1);
	//femas_tdspi->QryInvestor();

	//查询账户持仓情况
	//sleep(1);
	//femas_tdspi->QryInvestorPosition();

	//查询成交单
	//sleep(1);
	//femas_tdspi->QryTrade();

	//sleep(1);
	//femas_tdspi->OrderInsert();

	char input_c;
	char innner_input_c;
	/*order insert*/
	string order_InstrumentID;
	char order_CombOffsetFlag;
	char order_Direction;
	int order_Volume;
	double order_Price;
	string order_OrderRef;

	/*order action delete*/
	string action_ExchangeId;
	int action_int_ExchangeId;
	string action_OrderRef;
	string action_UserOrderActionLocalID;
	string action_OrderSysId;

	/*qry vars*/
	string qry_ExchangeId;
	string qry_InstrumentId;
	int qry_Flag;

#if 0
	printMenu();
	//测试下单，撤单
	while (1) {
		input_c = getchar();
		if (input_c == 'i') {
			cout << "Order Insert Operation" << endl;
			/************************************************************************/
			/* Code Below  Order Insert                                             */
			/************************************************************************/
			cout << "Please input instrumentID, such as cu1609, zn1701..." << endl;
			cin >> order_InstrumentID;
			cout << "Please input Comboffsetflag" << endl;
			cout << "0:Open 1:Close 2:ForceClose 3:CloseToday 4:CloseYesterday" << endl;
			cin >> order_CombOffsetFlag;
			cout << "Please input Direction" << endl;
			cout << "0:Buy 1:Sell" << endl;
			cin >> order_Direction;
			cout << "Please input Volume, such as 10, 50, 100..." << endl;
			cin >> order_Volume;
			cout << "Please input Price" << endl;
			cin >> order_Price;
			cout << "Please input OrderRef, such as 1, 2, 3 or user specific" << endl;
			cin >> order_OrderRef;

			cout << "order_InstrumentID = " << order_InstrumentID << endl;
			cout << "order_CombOffsetFlag = " << order_CombOffsetFlag << endl;
			cout << "order_Direction = " << order_Direction << endl;
			cout << "order_Volume = " << order_Volume << endl;
			cout << "order_Price = " << order_Price << endl;
			cout << "order_OrderRef = " << order_OrderRef << endl;

			femas_tdspi->OrderInsert(const_cast<char *>(order_InstrumentID.c_str()), order_CombOffsetFlag, order_Direction, order_Volume, order_Price, order_OrderRef);
			/************************************************************************/
			/* Code Finished                                                        */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'a') {
			cout << "Order Action Operation" << endl;
			/************************************************************************/
			/* Code Below  Order Action                                             */
			/************************************************************************/
			cout << "Pleas Input ExchanageID" << endl;
			cout << "1:SHFE 2:DCE 3:CZCE 4:CFFEX" << endl;
			cin >> action_int_ExchangeId;
			cout << "Pleas Input action_UserOrderLocalID" << endl;
			cin >> action_OrderRef;
			cout << "Pleas Input action_UserOrderActionLocalID" << endl;
			cin >> action_UserOrderActionLocalID;

			cout << "Pleas Input OrderSysId" << endl;
			cin.ignore(0x7fffffff, '\n');
			getline(cin, action_OrderSysId);
			//cin >> action_OrderSysId;

			switch (action_int_ExchangeId)
			{
			case 1:action_ExchangeId = "SHFE"; break;
			case 2:action_ExchangeId = "DCE"; break;
			case 3:action_ExchangeId = "CZCE"; break;
			case 4:action_ExchangeId = "CFFEX"; break;
			default:
				action_ExchangeId = "";
				break;
			}

			cout << "action_ExchangeId = " << action_ExchangeId << endl;
			cout << "action_UserOrderLocalID = " << action_OrderRef << endl;
			cout << "action_UserOrderActionLocalID = " << action_UserOrderActionLocalID << endl;
			cout << "action_OrderSysId = " << action_OrderSysId << endl;

			femas_tdspi->OrderAction(action_ExchangeId, action_OrderRef, action_OrderSysId, action_UserOrderActionLocalID);

			/************************************************************************/
			/* Code Finished                                                        */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'b') {
			cout << "拜拜~" << endl;
			break;
		}
		else if (input_c == 's') {
			//报单
			//femas_tdspi->OrderInsert("rb1801", '0', '0', 1, 3457, Utils::longlongtostr(femas_tdspi->generate_orderref()));
			//femas_tdspi->OrderInsert("rb1805", '0', '0', 1, 3290, Utils::longlongtostr(femas_tdspi->generate_orderref()));
			femas_tdspi->OrderInsert("rb1801", '0', '0', 1, 3366, Utils::longlongtostr(femas_tdspi->generate_orderref()));
			//femas_tdspi->OrderInsert("al1712", '0', '0', 1, 15575, Utils::longlongtostr(femas_tdspi->generate_orderref()));
			//sleep(1);


			/************************************************************************/
			/* code finished                                                                     */
			/************************************************************************/
			//printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'e') {
			cout << "Qry Exchange Operation" << endl;
			/************************************************************************/
			/* code below here                                                                     */
			/************************************************************************/

			femas_tdspi->QryExchange();

			/************************************************************************/
			/* code finished                                                                     */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'o') {
			cout << "Qry Order Operation" << endl;
			/************************************************************************/
			/* code below here                                                                     */
			/************************************************************************/

			femas_tdspi->QryOrder();

			/************************************************************************/
			/* code finished                                                                     */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'u') {
			cout << "Qry Investor Operation" << endl;
			/************************************************************************/
			/* code below here                                                                     */
			/************************************************************************/

			femas_tdspi->QryInvestor();

			/************************************************************************/
			/* code finished                                                                     */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'd') {
			cout << "Qry Trade Operation" << endl;
			/************************************************************************/
			/* code below here                                                                     */
			/************************************************************************/

			femas_tdspi->QryTrade();

			/************************************************************************/
			/* code finished                                                                     */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
		else if (input_c == 'w') {
			cout << "" << endl;
			/************************************************************************/
			/* code below here                                                                     */
			/************************************************************************/


			/************************************************************************/
			/* code finished                                                                     */
			/************************************************************************/
			printContinue();
			getchar();
			innner_input_c = getchar();
			if (innner_input_c == 'c') {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
			else {
				input_c = ' ';
				innner_input_c = ' ';
				printMenu();
				continue;
			}
		}
	}
#endif
	//sleep(1);
	femas_tdspi->Join();
	qdp_tdspi->Join();
	//sleep(1);
	//femas_tdspi->Release();
	return 0;
}
