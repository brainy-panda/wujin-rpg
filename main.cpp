#include <cstdio>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <map>
#include <deque>
#include <algorithm>
#include <limits>

// input format:
// <ORDER ID> <BUY | SELL> <QTY> <PRICE>
// <ORDER ID> REVISE <QTY> <PRICE>
// <ORDER ID> CANCEL

// output messages:
// <BUY | SELL> <QTY> <PRICE> <ORDER ID>
// REVISE <QTY> <PRICE> <ORDER ID>
// CANCEL <ORDER ID> <QTY>
// TRADE <AGGRESSIVE ID> <PASSIVE ID> <QTY> <PRICE>

struct Order
{
    Order(uint64_t o, int32_t p, int32_t q) : orderId(o), price(p), qty(q) {}
    uint64_t orderId;
    int32_t price;
    int32_t qty;
};

struct SideLevel
{
    SideLevel() = default;
    SideLevel(bool ib, int32_t p) : isBuy(ib), price(p) {}
    bool isBuy;
    int32_t price;
};

using LevelQueue = std::deque<Order>;

struct Market
{
    void AddOrder(uint64_t orderId, bool isBuy, int32_t qty, int32_t price)
    {
	// printf("DEBUG AddOrder: orderId=%lu isBuy=%d qty=%d price=%d\n", orderId, isBuy, qty, price);
	LevelQueue* levelQueue;
	if (isBuy)
	    levelQueue = &_bidLevels[price]; // construct if not exist
	else
	    levelQueue = &_askLevels[price];
	levelQueue->emplace_back(orderId, price, qty);
	auto sideLevel = SideLevel(isBuy, price);
	_idToSideLevel[orderId] = sideLevel;

	const char* side = isBuy ? "BUY" : "SELL";
	printf("%s %d %d %lu\n", side, qty, price, orderId);

	MatchOrders(isBuy);
    }

    void ReviseOrder(uint64_t orderId, int32_t qty, int32_t price)
    {
	// printf("DEBUG ReviseOrder: orderId=%lu qty=%d price=%d\n", orderId, qty, price);
	auto it = _idToSideLevel.find(orderId);
	if (it != _idToSideLevel.end())
	{
	    SideLevel& sideLevel = it->second;
	    CancelOrder(sideLevel, orderId);
	    AddOrder(orderId, sideLevel.isBuy, qty, price);
	    sideLevel.price = price; // cannot change side with revise
	}
    }

    void CancelOrder(SideLevel sideLevel, uint64_t orderId)
    {
	LevelQueue* levelQueue = nullptr;
	if (sideLevel.isBuy)
	{
	    auto bidLevelIt = _bidLevels.find(sideLevel.price);
	    if (bidLevelIt != _bidLevels.end())
		levelQueue = &bidLevelIt->second;
	}
	else
	{
	    auto askLevelIt = _askLevels.find(sideLevel.price);
	    if (askLevelIt != _askLevels.end())
		levelQueue = &askLevelIt->second;
	}
	if (!levelQueue)
	    return; // should never happen
	auto orderIt = std::find_if(levelQueue->begin(), levelQueue->end(), [orderId](const Order& order)
				    {
					return order.orderId == orderId;
				    });
	int32_t qtyCancelled = orderIt->qty;
	levelQueue->erase(orderIt);
	printf("CANCEL %lu %d\n", orderId, qtyCancelled);
    }

    void CancelOrder(uint64_t orderId)
    {
	// printf("DEBUG CancelOrder: orderId=%lu\n", orderId);
	auto it = _idToSideLevel.find(orderId);
	if (it != _idToSideLevel.end())
	{
	    SideLevel sideLevel = it->second;
	    CancelOrder(sideLevel, orderId);
	    _idToSideLevel.erase(it);
	}
    }

    void MatchOrders(bool aggressorIsBuy)
    {
	// match all orders which can match and print them
	while (TryMatchBests(aggressorIsBuy))
	    continue;
    }

    bool TryMatchBests(bool aggressorIsBuy)
    {
	auto bestBidLevelIt = _bidLevels.begin();
	auto bestAskLevelIt = _askLevels.begin();
	if (bestBidLevelIt == _bidLevels.end())
	    return false;
	if (bestAskLevelIt == _askLevels.end())
	    return false;
	int32_t bestBidPrice = bestBidLevelIt->first;
	int32_t bestAskPrice = bestAskLevelIt->first;
	if (bestBidPrice < bestAskPrice)
	    return false;

	// else, we can match
	LevelQueue& bestBidQueue = bestBidLevelIt->second;
	LevelQueue& bestAskQueue = bestAskLevelIt->second;
	Order& bestBidFrontOrder = bestBidQueue.front();
	Order& bestAskFrontOrder = bestAskQueue.front();
	uint64_t aggrId = aggressorIsBuy ? bestBidFrontOrder.orderId : bestAskFrontOrder.orderId;
	uint64_t passiveId = aggressorIsBuy ? bestAskFrontOrder.orderId : bestBidFrontOrder.orderId;
	int32_t tradePrice = aggressorIsBuy ? bestAskFrontOrder.price : bestBidFrontOrder.price;
	if (bestBidFrontOrder.qty > bestAskFrontOrder.qty)
	{
	    int32_t tradeQty = bestAskFrontOrder.qty;
	    bestBidFrontOrder.qty -= tradeQty;
	    uint64_t idToDelete = bestAskFrontOrder.orderId;
	    bestAskQueue.pop_front();
	    _idToSideLevel.erase(idToDelete);
	    printf("TRADE %lu %lu %d %d\n", aggrId, passiveId, tradeQty, tradePrice);
	}
	else if (bestBidFrontOrder.qty < bestAskFrontOrder.qty)
	{
	    int32_t tradeQty = bestBidFrontOrder.qty;
	    bestAskFrontOrder.qty -= tradeQty;
	    uint64_t idToDelete = bestBidFrontOrder.orderId;
	    bestBidQueue.pop_front();
	    _idToSideLevel.erase(idToDelete);
	    printf("TRADE %lu %lu %d %d\n", aggrId, passiveId, tradeQty, tradePrice);
	}
	else // they're equal
	{
	    int32_t tradeQty = bestBidFrontOrder.qty; // bid and ask qty same anyway
	    bestBidQueue.pop_front();
	    bestAskQueue.pop_front();
	    _idToSideLevel.erase(bestBidFrontOrder.orderId);
	    _idToSideLevel.erase(bestAskFrontOrder.orderId);
	    printf("TRADE %lu %lu %d %d\n", aggrId, passiveId, tradeQty, tradePrice);
	}
	if (bestAskQueue.empty())
	    _askLevels.erase(bestAskLevelIt);
	if (bestBidQueue.empty())
	    _bidLevels.erase(bestBidLevelIt);
	return true;
	// TRADE <AGGRESSIVE ID> <PASSIVE ID> <QTY> <PRICE>
    }

    std::unordered_map<uint64_t, SideLevel> _idToSideLevel;

    // bid levels are in descending order, front is best/highest
    std::map<int32_t, LevelQueue, std::greater<int32_t>> _bidLevels;
    // ask levels are in ascending order, front is best/lowest
    std::map<int32_t, LevelQueue> _askLevels;
};

int main()
{
    using namespace std;
    Market market;

    uint64_t orderId;
    int32_t price;
    int32_t qty;
    std::string command;
    while (std::cin >> orderId)
    {
	std::cin >> command;
	if (command == "BUY")
	{
	    std::cin >> qty;
	    std::cin >> price;
	    market.AddOrder(orderId, true, qty, price);
	    std::cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	else if (command == "SELL")
	{
	    std::cin >> qty;
	    std::cin >> price;
	    market.AddOrder(orderId, false, qty, price);
	    std::cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	else if (command == "REVISE")
	{
	    std::cin >> qty;
	    std::cin >> price;
	    market.ReviseOrder(orderId, qty, price);
	    std::cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	else if (command == "CANCEL")
	{
	    market.CancelOrder(orderId);
	    std::cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	else
	    printf("could not parse command\n");
    }
}
