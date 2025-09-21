import yfinance as yf
import csv
import sys

# --- List of Stock Symbols to Fetch ---
# You can customize this list with any valid stock tickers.
STOCK_SYMBOLS = [
    "AAPL", "GOOGL", "MSFT", "AMZN", "TSLA", 
    "NVDA", "JPM", "V", "JNJ", "WMT"
]

# --- Name of the output CSV file ---
CSV_FILENAME = "stock_info.csv"

def fetch_stock_data():
    """
    Fetches stock data for a list of symbols from Yahoo Finance 
    and writes it to a CSV file.
    """
    print("[Python] Starting to fetch stock data...")
    
    stock_data_list = []
    
    for symbol in STOCK_SYMBOLS:
        try:
            print(f"[Python] Fetching data for {symbol}...")
            stock = yf.Ticker(symbol)
            info = stock.info

            # --- Data Extraction ---
            # We use .get() to avoid errors if a key is missing, providing 'N/A' as a default.
            prev_close = info.get('previousClose', 'N/A')
            day_range = f"{info.get('dayLow', 'N/A')} - {info.get('dayHigh', 'N/A')}"
            year_range = f"{info.get('fiftyTwoWeekLow', 'N/A')} - {info.get('fiftyTwoWeekHigh', 'N/A')}"
            market_cap = info.get('marketCap', 'N/A')
            avg_volume = info.get('averageVolume', 'N/A')
            
            # Dividend yield is a ratio, so we multiply by 100 to get a percentage
            div_yield = info.get('dividendYield')
            div_yield_percent = f"{div_yield * 100:.2f}%" if isinstance(div_yield, (int, float)) else 'N/A'

            pe_ratio = info.get('trailingPE', 'N/A')

            # --- Append to our list for CSV writing ---
            stock_data_list.append([
                symbol, prev_close, day_range, year_range, 
                market_cap, avg_volume, div_yield_percent, pe_ratio
            ])

        except Exception as e:
            print(f"[Python ERROR] Could not fetch data for {symbol}: {e}", file=sys.stderr)
            # Add a row with 'N/A' for all fields if the ticker fails
            stock_data_list.append([symbol] + ['N/A'] * 7)

    return stock_data_list

def write_to_csv(data):
    """
    Writes the collected stock data to a CSV file.
    """
    header = [
        "Symbol", "Prev Close", "Day Range", "Year Range", 
        "Market Cap", "Avg Volume", "Div Yield", "P/E Ratio"
    ]
    
    try:
        with open(CSV_FILENAME, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(header)
            writer.writerows(data)
        print(f"[Python] Successfully wrote data to {CSV_FILENAME}")
    except IOError as e:
        print(f"[Python ERROR] Could not write to file {CSV_FILENAME}: {e}", file=sys.stderr)
        sys.exit(1) # Exit with an error code

if __name__ == "__main__":
    data_to_write = fetch_stock_data()
    if data_to_write:
        write_to_csv(data_to_write)
    else:
        print("[Python] No data was fetched. CSV file not created.", file=sys.stderr)
        sys.exit(1) # Exit with an error code
