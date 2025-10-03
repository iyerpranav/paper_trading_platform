import yfinance as yf
import sqlite3
import sys
from datetime import datetime

# --- List of Stock Symbols to Fetch ---
STOCK_SYMBOLS = [
    "AAPL", "GOOGL", "MSFT", "AMZN", "TSLA",
    "NVDA", "JPM", "V", "JNJ", "WMT"
]

DATABASE = "stock_portfolio.db"

def connect_db():
    """
    Connect to the SQLite database and create Stock + MarketData tables if not exist.
    """
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()

    # Create Stock table
    c.execute("""
    CREATE TABLE IF NOT EXISTS Stock (
        StockID INTEGER PRIMARY KEY AUTOINCREMENT,
        Symbol TEXT NOT NULL UNIQUE,
        CompanyName TEXT NOT NULL,
        MarketCap INTEGER,
        AvgVolume INTEGER,
        DividendYield REAL,
        PERatio REAL,
        FiftyTwoWeekLow REAL,
        FiftyTwoWeekHigh REAL,
        DayLow REAL,
        DayHigh REAL,
        PreviousClose REAL
    )
    """)

    # Create MarketData table
    c.execute("""
    CREATE TABLE IF NOT EXISTS MarketData (
        MarketDataID INTEGER PRIMARY KEY AUTOINCREMENT,
        StockID INTEGER NOT NULL,
        Price REAL NOT NULL,
        Volume INTEGER,
        Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (StockID) REFERENCES Stock(StockID)
    )
    """)
    conn.commit()
    return conn


def upsert_stock(conn, symbol, info):
    """
    Insert or update a stock's fundamentals in the Stock table.
    """
    c = conn.cursor()
    c.execute("""
        INSERT INTO Stock (
            Symbol, CompanyName, MarketCap, AvgVolume,
            DividendYield, PERatio, FiftyTwoWeekLow,
            FiftyTwoWeekHigh, DayLow, DayHigh, PreviousClose
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(Symbol) DO UPDATE SET
            CompanyName=excluded.CompanyName,
            MarketCap=excluded.MarketCap,
            AvgVolume=excluded.AvgVolume,
            DividendYield=excluded.DividendYield,
            PERatio=excluded.PERatio,
            FiftyTwoWeekLow=excluded.FiftyTwoWeekLow,
            FiftyTwoWeekHigh=excluded.FiftyTwoWeekHigh,
            DayLow=excluded.DayLow,
            DayHigh=excluded.DayHigh,
            PreviousClose=excluded.PreviousClose
    """, (
        symbol,
        info.get('shortName') or symbol,
        info.get('marketCap'),
        info.get('averageVolume'),
        info.get('dividendYield'),
        info.get('trailingPE'),
        info.get('fiftyTwoWeekLow'),
        info.get('fiftyTwoWeekHigh'),
        info.get('dayLow'),
        info.get('dayHigh'),
        info.get('previousClose')
    ))
    conn.commit()

    # Return StockID
    c.execute("SELECT StockID FROM Stock WHERE Symbol=?", (symbol,))
    return c.fetchone()[0]


def insert_market_data(conn, stock_id, price, volume, timestamp=None):
    """
    Insert time-series price/volume into MarketData.
    """
    if timestamp is None:
        timestamp = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")
    c = conn.cursor()
    c.execute("""
        INSERT INTO MarketData (StockID, Price, Volume, Timestamp)
        VALUES (?, ?, ?, ?)
    """, (stock_id, price, volume, timestamp))
    conn.commit()


def fetch_stock_data():
    """
    Fetch stock data from yfinance.
    """
    print("[Python] Fetching stock data...")
    stock_data = {}
    for symbol in STOCK_SYMBOLS:
        try:
            print(f"[Python] {symbol} ...")
            stock = yf.Ticker(symbol)
            stock_data[symbol] = stock.info
        except Exception as e:
            print(f"[ERROR] Failed fetching {symbol}: {e}", file=sys.stderr)
            stock_data[symbol] = {}
    return stock_data


def update_database(data):
    """
    Update Stock fundamentals and insert MarketData rows.
    """
    conn = connect_db()
    for symbol, info in data.items():
        if not info:
            continue
        try:
            stock_id = upsert_stock(conn, symbol, info)

            price = info.get("regularMarketPrice") or info.get("previousClose")
            volume = info.get("volume") or info.get("averageVolume")

            if price is not None and volume is not None:
                insert_market_data(conn, stock_id, price, volume)
        except Exception as e:
            print(f"[DB ERROR] {symbol}: {e}", file=sys.stderr)

    conn.close()


if __name__ == "__main__":
    stock_data = fetch_stock_data()
    if stock_data:
        update_database(stock_data)
        print("[Python] Database updated with latest stock + market data.")
    else:
        print("[Python] No stock data fetched.", file=sys.stderr)
        sys.exit(1)
