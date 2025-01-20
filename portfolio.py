import csv
import requests
from bs4 import BeautifulSoup

def get_current_price(symbol):
    """
    Retrieve the current price of a stock from Google Finance.

    Args:
    symbol (str): The stock symbol.

    Returns:
    str: The current price of the stock.
    """
    base_url = "https://www.google.com/finance"
    index = "NSE"  # You may need to change this based on the stock exchange
    language = "en"
    target_url = f"{base_url}/quote/{symbol}:{index}?hl={language}"

    try:
        # Make an HTTP request with timeout
        page = requests.get(target_url, timeout=10)
        page.raise_for_status()  # Raise an exception for any HTTP errors
    except requests.RequestException as e:
        print(f"Error fetching data for {symbol}: {e}")
        return None

    # Use an HTML parser to grab the content from the page
    soup = BeautifulSoup(page.content, "html.parser")

    # Find the element containing the current price
    current_price_element = soup.find("div", class_="YMlKec fxKbKc")
    if current_price_element:
        return current_price_element.text.strip()
    else:
        return None

# Read existing portfolio data from the CSV file
portfolio_data = []
with open('portfolio.csv', 'r', newline='', encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        portfolio_data.append(row)

# Update the current price for each stock in the portfolio
for stock in portfolio_data:
    current_price = get_current_price(stock['Stock Symbol'])
    if current_price:
        # Remove commas and rupees symbol from the current price
        price_without_rupees = current_price.replace(',', '').replace('₹', '')
        stock['Current Price'] = price_without_rupees
    else:
        print(f"Failed to retrieve current price for {stock['Stock Symbol']}")

# Write the updated portfolio data back to the CSV file
with open('portfolio.csv', 'w', newline='', encoding='utf-8') as csvfile:
    fieldnames = portfolio_data[0].keys() if portfolio_data else []
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

    # Write header row
    writer.writeheader()

    # Write updated portfolio data
    writer.writerows(portfolio_data)

print("Current prices have been updated in the portfolio.")
