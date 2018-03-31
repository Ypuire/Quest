#include "Merchant.h"

bool Merchant::valid()
{
	if (m_buy_price_percent < 0)
		return false;
	if (m_sell_price_percent < 0)
		return false;
	return true;
}