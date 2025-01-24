import csv
import requests
from bs4 import BeautifulSoup

def get_stock_info(symbol):
    """
    Retrieve stock information from Google Finance for a given stock symbol.

    Args:
    symbol (str): The stock symbol to retrieve information for.

    Returns:
    dict: A dictionary containing the stock information.
    """
    base_url = "https://www.google.com/finance"
    index = "NSE"
    language = "en"
    target_url = f"{base_url}/quote/{symbol}:{index}?hl={language}"

    # Make an HTTP request with timeout
    page = requests.get(target_url, timeout=10)

    # Use an HTML parser to grab the content from "page"
    soup = BeautifulSoup(page.content, "html.parser")

    # Create a dictionary to store the stock information
    stock_data = {
        "Symbol": symbol,
        "Previous close": "",
        "Day range": "",
        "Year range": "",
        "Market cap": "",
        "Avg Volume": "",
        "Dividend yield": "",
        "P/E ratio": ""
    }

    # Find specific elements containing the required information
    for key in stock_data:
        element = soup.find("div", string=key)
        if element:
            value_element = element.find_next("div", class_="P6K39c")
            if value_element:
                # Remove commas and rupees symbol from the value
                value = value_element.text.strip().replace(",", "").replace("₹", "")
                stock_data[key] = value

    return stock_data

# Read stock symbols from the .txt file
with open('stock_symbols.txt', 'r', encoding='utf-8') as file:
    stock_symbols_list = file.read().splitlines()

# Read existing data from the CSV file and keep only the header row
existing_data = []
with open('stock_info.csv', 'r', newline='', encoding='utf-8') as csvfile:
    reader = csv.reader(csvfile)
    header_row = next(reader)
    existing_data.append(header_row)

# Remove 'Primary exchange' from the header row if it exists
if 'Primary exchange' in header_row:
    header_row.remove('Primary exchange')

# Write stock information to the CSV file
with open('stock_info.csv', 'w', newline='', encoding='utf-8') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=header_row)

    # Write header row
    writer.writeheader()

    # Write existing data (excluding the header row)
    for row in existing_data[1:]:
        writer.writerow(dict(zip(header_row, row)))

    # Write new data
    for stock_symbol in stock_symbols_list:
        stock_info = get_stock_info(stock_symbol)
        writer.writerow(stock_info)

print("Stock information has been updated in stock_info.csv")
