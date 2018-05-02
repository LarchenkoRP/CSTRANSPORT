/**
@author Larchenko Roman
@email  LarchenkoRP@mail.ru
@date   01/05/2018
*/

#pragma once

#include <cstdlib>
#include <iostream>
#include <vector>
#include <functional>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

using boost::asio::ip::udp;



/**
@class SessionIO
����� ������������ ������ ������������ ������/�������� ���������� �� ��������� UDP.
*/
class SessionIO
{
public:
	/*!
	* \brief ����������� �� ���������.
	*/
	SessionIO();

	/*!
	* \brief ����������� ���������� �� ���������.
	*/
	virtual ~SessionIO();

	/*!
	* \brief ������� ������� �������.
	*/
	void Run();

private:

	unsigned int DefiningNode_;				/// ����������� ������ ���� ( ������� )
	const uint16_t version_ = 1;			/// ������ ����
	const char * host_port_ = { "9001" };	/// ���� ����� �� ���������
	const char * server_port_ = { "6000" }; /// ���� ����������� ������� �� ���������
	static bool OneReg;						/// ���� ����������� 
	
	/// ��������� ����
	struct PacketNode
	{
		std::string hash; /// ���
		std::string key;  /// ����
		std::string ip;	  /// IP �����
		std::string port; /// ���� 
	};

	std::string ServerHash_; /// ��� �������
	std::string ServerKey_;  /// ���� �������
	std::string MyHash_;     /// ��� ������� ����
	std::string MyPublicKey_;/// ��������� ���� ������� ����

	std::vector<std::string> �onfidantNodes_; /// ������ ���������� �����
	std::string GeneralNode_;                 /// ������� ����

	/// ������������ ������� ����
	enum LevelNodes {
		Normal = 0x00,    /// ������� ����
		�onfidant = 0x01, /// ���������� ����
		Main = 0x02,      /// ������� ����
		Write = 0x03      /// ������� ����
	};

	/// ������������ ������
	enum CommandList {
		Registration = 0x01,         /// �����������
		UnRegistration = 0x02,		 /// �������������� 
		Redirect = 0x03,			 /// ����������
		GetHash = 0x04,              /// ������� ���
		SendHash = 0x05,		     /// ��������� ���
		SendTransaction = 0x06,      /// ��������� ����������
		GetTransaction = 0x07,       /// ������� ����������
		SendTransactionList = 0x08,  /// ��������� �������������� ����
		GetTransactionList = 0x09,   /// ������� �������������� ����
		SendVector = 0x10,           /// ��������� ������
		GetVector = 0x11,			 /// ������� ������
		SendMatrix = 0x12,           /// ��������� �������
		GetMatrix = 0x13,            /// ������� �������
		SendBlock = 0x14,            /// ��������� ���� ������
		GetHashAll = 0x015,          /// ������ ���(�) �� ���� �����
		SendIpTable = 0x016          /// ��������� ������ ���������� ����� � �������� ���� �����
	};

	/// ������������ ���������
	enum SubCommandList {
		RegistrationLevelNode = 0x01, /// ������� ������ ���������� � �������� ����
		GiveHash = 0x02,              /// ������ �� ���
		GetBlock = 0x03,			  /// ������ �� ���� ������
	};

	enum { max_length = 65447, hash_length = 40, publicKey_length = 40 };

	/// ��������� ������ ������/�������� ����������
	struct Packet
	{
		uint8_t command;                        /// �������
		uint8_t subcommand;						/// ����������
		uint8_t version;						/// ������
		uint16_t header;						/// ����� ���������
		uint16_t countHeader;					/// ���������� ����������
		uint8_t hash[hash_length];              /// ��� �����������/������������ ����
		uint8_t publickKey[publicKey_length];   /// ��������� ���� �����������/������������ ����
		uint8_t data[max_length];               /// ������
	}RecvBuffer, SendBuffer;

	/*!
	* \brief ����� ������������ ���������� ����������.
	* \return ������ ��������� ���������� ������ ������/�������� ����������
	*/
	constexpr std::size_t CalcSum() const;
	unsigned int SizePacketHeader;               /// ������ ��������� ���������� ������ ������/�������� ����������

	boost::asio::io_service io_service_client_;  /// ������ �������
	boost::asio::io_service io_service_server_;  /// ������ �������
	
	udp::socket * InputServiceSocket_;           /// ����� ������� ���������� 
	udp::endpoint InputServiceRecvEndpoint_;	 /// ������� ����� ������ ����������
	udp::endpoint InputServiceSendEndpoint_;     /// ������� ����� �������� ���������� 
	udp::resolver InputServiceResolver_;         /// �������� �������

	udp::socket * OutputServiceSocket_;          /// ����� �������� ���������� 
	udp::endpoint OutputServiceRecvEndpoint_;    /// ������� ����� ������ ����������
	udp::endpoint OutputServiceSendEndpoint_;    /// ������� ����� ������ ����������
	udp::endpoint OutputServiceServerEndpoint_;  /// ������� ����� ����������� �������
	udp::resolver OutputServiceResolver_;		 /// �������� �������

	boost::circular_buffer<PacketNode> NodesRing_;   /// ��������� ����� �������� �����
	boost::circular_buffer<std::string> BackData_;	 /// ��������� ����� �������� ���������� ����������

	boost::detail::spinlock SpinLock_;               /// C���-����������
	boost::property_tree::ptree config;              /// ����� �������������
	boost::asio::deadline_timer timer;				 /// ������


	
	/*!
	* \brief ����� ������������ ���������� ����������.
	*/
	void Initialization();

	/*!
	* \brief ����� ������ ����������.
	*/
	void InputServiceHandleReceive(const boost::system::error_code& error, std::size_t bytes_transferred);

	/*!
	* \brief ����� ������ ����������.
	*/
	void OutputServiceHandleSend(const boost::system::error_code& error, std::size_t bytes_transferred);

	/*!
	* \brief ����� ������ ����������.
	*/
	void OutputServiceSendCommand(const Packet & pack, unsigned int lenData);

	/*!
	* \brief ����� ������ ����������.
	*/
	void SolverSendData(char * buffer, unsigned int buf_size, char * ip_buffer, unsigned int ip_size, unsigned int cmd);

	/*!
	* \brief ����� ������ ���.
	*/
	void SolverSendHash(char * buffer, unsigned int buf_size, char * ip_buffer, unsigned int ip_size);

	/*!
	* \brief ����� ������� ��� ���� �����.
	*/
	void SolverGetHashAll();

	/*!
	* \brief ����� ������� ��� ������� ����.
	*/
	void SolverGiveHash(std::size_t bytes_transferred);

	/*!
	* \brief ����� �������� ����������.
	*/
	void SolverSendTransaction(const char * data, unsigned size);

	/*!
	* \brief ����� �������� ����������.
	*/
	void SolverSendTransactionList(const char * data, unsigned size);

	/*!
	* \brief ����� �������� �������.
	*/
	void SolverSendVector(const char * data, unsigned size);

	/*!
	* \brief ����� �������� �������.
	*/
	void SolverSendMatrix(const char * data, unsigned size);

	/*!
	* \brief ����� �������� ����� ������.
	*/
	void SolverSendBlock(const char * data, unsigned size);

	/*!
	* \brief ����� ��������� ���������� � �������� ����.
	*/
	void GenTableRegistrationLevelNode(const char * data, unsigned size);

	/*!
	* \brief ����� ��������� ���������� ���.
	*/
	void GenerationHash();

	/*!
	* \brief ����� ������� ���� ���������.
	*/
	void Begin();

	/*!
	* \brief ����� ��������� ���������� �����.
	*/
	bool RunRedirect(std::size_t bytes_transferred);
	
	/*!
	* \brief ����� �������������� �� ���������� �������.
	*/
	void InRegistrationNode();

	/*!
	* \brief ����� ����������� ���������� � �������� ����.
	*/
	void InRegistrationLevelNode(std::size_t bytes_transferred);

	/*!
	* \brief ����� ����������� �� ���������� �������.
	*/
	void RegistrationToServer();

	void DefiningNode(unsigned int init);
	
};

