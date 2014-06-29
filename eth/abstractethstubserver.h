/**
 * THIS FILE IS GENERATED BY jsonrpcstub, DO NOT CHANGE IT!!!!!
 */

#ifndef _ABSTRACTETHSTUBSERVER_H_
#define _ABSTRACTETHSTUBSERVER_H_

#include <jsonrpc/rpc.h>

class AbstractEthStubServer : public jsonrpc::AbstractServer<AbstractEthStubServer>
{
    public:
        AbstractEthStubServer(jsonrpc::AbstractServerConnector* conn) :
            jsonrpc::AbstractServer<AbstractEthStubServer>(conn) 
        {
            this->bindAndAddMethod(new jsonrpc::Procedure("balanceAt", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "a",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::balanceAtI);
            this->bindAndAddMethod(new jsonrpc::Procedure("block", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, "a",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::blockI);
            this->bindAndAddMethod(new jsonrpc::Procedure("check", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_ARRAY, "a",jsonrpc::JSON_ARRAY, NULL), &AbstractEthStubServer::checkI);
            this->bindAndAddMethod(new jsonrpc::Procedure("coinbase", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING,  NULL), &AbstractEthStubServer::coinbaseI);
            this->bindAndAddMethod(new jsonrpc::Procedure("create", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "bCode",jsonrpc::JSON_STRING,"sec",jsonrpc::JSON_STRING,"xEndowment",jsonrpc::JSON_STRING,"xGas",jsonrpc::JSON_STRING,"xGasPrice",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::createI);
            this->bindAndAddMethod(new jsonrpc::Procedure("gasPrice", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING,  NULL), &AbstractEthStubServer::gasPriceI);
            this->bindAndAddMethod(new jsonrpc::Procedure("isContractAt", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_BOOLEAN, "a",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::isContractAtI);
            this->bindAndAddMethod(new jsonrpc::Procedure("isListening", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_BOOLEAN,  NULL), &AbstractEthStubServer::isListeningI);
            this->bindAndAddMethod(new jsonrpc::Procedure("isMining", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_BOOLEAN,  NULL), &AbstractEthStubServer::isMiningI);
            this->bindAndAddMethod(new jsonrpc::Procedure("key", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING,  NULL), &AbstractEthStubServer::keyI);
            this->bindAndAddMethod(new jsonrpc::Procedure("keys", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_ARRAY,  NULL), &AbstractEthStubServer::keysI);
            this->bindAndAddMethod(new jsonrpc::Procedure("lastBlock", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT,  NULL), &AbstractEthStubServer::lastBlockI);
            this->bindAndAddMethod(new jsonrpc::Procedure("lll", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "s",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::lllI);
            this->bindAndAddMethod(new jsonrpc::Procedure("peerCount", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_INTEGER,  NULL), &AbstractEthStubServer::peerCountI);
            this->bindAndAddMethod(new jsonrpc::Procedure("procedures", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_ARRAY,  NULL), &AbstractEthStubServer::proceduresI);
            this->bindAndAddMethod(new jsonrpc::Procedure("secretToAddress", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "a",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::secretToAddressI);
            this->bindAndAddMethod(new jsonrpc::Procedure("sim_call", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "aDest",jsonrpc::JSON_STRING,"aOrigin",jsonrpc::JSON_STRING,"aSend",jsonrpc::JSON_STRING,"bData",jsonrpc::JSON_STRING,"xGas",jsonrpc::JSON_STRING,"xGasPrice",jsonrpc::JSON_STRING,"xValue",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::sim_callI);
            this->bindAndAddMethod(new jsonrpc::Procedure("storageAt", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "a",jsonrpc::JSON_STRING,"x",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::storageAtI);
            this->bindAndAddMethod(new jsonrpc::Procedure("transact", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, "aDest",jsonrpc::JSON_STRING,"bData",jsonrpc::JSON_STRING,"sec",jsonrpc::JSON_STRING,"xGas",jsonrpc::JSON_STRING,"xGasPrice",jsonrpc::JSON_STRING,"xValue",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::transactI);
            this->bindAndAddMethod(new jsonrpc::Procedure("txCountAt", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "a",jsonrpc::JSON_STRING, NULL), &AbstractEthStubServer::txCountAtI);

        }
        
        inline virtual void balanceAtI(const Json::Value& request, Json::Value& response) 
        {
            response = this->balanceAt(request["a"].asString());
        }

        inline virtual void blockI(const Json::Value& request, Json::Value& response) 
        {
            response = this->block(request["a"].asString());
        }

        inline virtual void checkI(const Json::Value& request, Json::Value& response) 
        {
            response = this->check(request["a"]);
        }

        inline virtual void coinbaseI(const Json::Value& request, Json::Value& response) 
        {
            response = this->coinbase();
        }

        inline virtual void createI(const Json::Value& request, Json::Value& response) 
        {
            response = this->create(request["bCode"].asString(), request["sec"].asString(), request["xEndowment"].asString(), request["xGas"].asString(), request["xGasPrice"].asString());
        }

        inline virtual void gasPriceI(const Json::Value& request, Json::Value& response) 
        {
            response = this->gasPrice();
        }

        inline virtual void isContractAtI(const Json::Value& request, Json::Value& response) 
        {
            response = this->isContractAt(request["a"].asString());
        }

        inline virtual void isListeningI(const Json::Value& request, Json::Value& response) 
        {
            response = this->isListening();
        }

        inline virtual void isMiningI(const Json::Value& request, Json::Value& response) 
        {
            response = this->isMining();
        }

        inline virtual void keyI(const Json::Value& request, Json::Value& response) 
        {
            response = this->key();
        }

        inline virtual void keysI(const Json::Value& request, Json::Value& response) 
        {
            response = this->keys();
        }

        inline virtual void lastBlockI(const Json::Value& request, Json::Value& response) 
        {
            response = this->lastBlock();
        }

        inline virtual void lllI(const Json::Value& request, Json::Value& response) 
        {
            response = this->lll(request["s"].asString());
        }

        inline virtual void peerCountI(const Json::Value& request, Json::Value& response) 
        {
            response = this->peerCount();
        }

        inline virtual void proceduresI(const Json::Value& request, Json::Value& response) 
        {
            response = this->procedures();
        }

        inline virtual void secretToAddressI(const Json::Value& request, Json::Value& response) 
        {
            response = this->secretToAddress(request["a"].asString());
        }

        inline virtual void sim_callI(const Json::Value& request, Json::Value& response) 
        {
            response = this->sim_call(request["aDest"].asString(), request["aOrigin"].asString(), request["aSend"].asString(), request["bData"].asString(), request["xGas"].asString(), request["xGasPrice"].asString(), request["xValue"].asString());
        }

        inline virtual void storageAtI(const Json::Value& request, Json::Value& response) 
        {
            response = this->storageAt(request["a"].asString(), request["x"].asString());
        }

        inline virtual void transactI(const Json::Value& request, Json::Value& response) 
        {
            response = this->transact(request["aDest"].asString(), request["bData"].asString(), request["sec"].asString(), request["xGas"].asString(), request["xGasPrice"].asString(), request["xValue"].asString());
        }

        inline virtual void txCountAtI(const Json::Value& request, Json::Value& response) 
        {
            response = this->txCountAt(request["a"].asString());
        }


        virtual std::string balanceAt(const std::string& a) = 0;
        virtual Json::Value block(const std::string& a) = 0;
        virtual Json::Value check(const Json::Value& a) = 0;
        virtual std::string coinbase() = 0;
        virtual std::string create(const std::string& bCode, const std::string& sec, const std::string& xEndowment, const std::string& xGas, const std::string& xGasPrice) = 0;
        virtual std::string gasPrice() = 0;
        virtual bool isContractAt(const std::string& a) = 0;
        virtual bool isListening() = 0;
        virtual bool isMining() = 0;
        virtual std::string key() = 0;
        virtual Json::Value keys() = 0;
        virtual Json::Value lastBlock() = 0;
        virtual std::string lll(const std::string& s) = 0;
        virtual int peerCount() = 0;
        virtual Json::Value procedures() = 0;
        virtual std::string secretToAddress(const std::string& a) = 0;
        virtual std::string sim_call(const std::string& aDest, const std::string& aOrigin, const std::string& aSend, const std::string& bData, const std::string& xGas, const std::string& xGasPrice, const std::string& xValue) = 0;
        virtual std::string storageAt(const std::string& a, const std::string& x) = 0;
        virtual Json::Value transact(const std::string& aDest, const std::string& bData, const std::string& sec, const std::string& xGas, const std::string& xGasPrice, const std::string& xValue) = 0;
        virtual std::string txCountAt(const std::string& a) = 0;

};
#endif //_ABSTRACTETHSTUBSERVER_H_
