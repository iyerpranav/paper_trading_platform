import streamlit as st
import pandas as pd
import requests
import json

# --- API Configuration ---
API_BASE_URL = "http://localhost:8080"

# --- Page Configuration ---
st.set_page_config(
    page_title="Stock Portfolio Manager",
    page_icon="💹",
    layout="wide"
)

# --- Session State Management ---
if 'logged_in' not in st.session_state:
    st.session_state.logged_in = False
    st.session_state.username = ""
    st.session_state.user_id = None

# --- API Client Functions ---
def api_login(username, password):
    try:
        response = requests.post(f"{API_BASE_URL}/login", json={"username": username, "password": password})
        if response.status_code == 200:
            return response.json()
    except requests.exceptions.ConnectionError:
        return {"success": False, "message": "Connection Error: Is the C++ backend server running?"}
    return None

def api_get_portfolio(user_id):
    try:
        response = requests.get(f"{API_BASE_URL}/portfolio/{user_id}")
        if response.status_code == 200:
            data = response.json()
            balance = data.get('fundBalance', 0.0)
            stocks = data.get('stocks', [])
            return balance, pd.DataFrame(stocks)
    except requests.exceptions.ConnectionError:
        st.error("Connection Error: Could not fetch portfolio.")
    return 0.0, pd.DataFrame()

def api_execute_transaction(user_id, symbol, quantity, price, trans_type):
    payload = {
        "userId": user_id,
        "symbol": symbol,
        "quantity": quantity,
        "price": price,
        "type": trans_type
    }
    try:
        response = requests.post(f"{API_BASE_URL}/transaction", json=payload)
        if response.status_code == 200:
            return response.json()
    except requests.exceptions.ConnectionError:
        return {"success": False, "message": "Connection Error."}
    return {"success": False, "message": "An unknown error occurred."}
    
def api_get_all_stocks():
    try:
        response = requests.get(f"{API_BASE_URL}/stocks")
        if response.status_code == 200:
            return pd.DataFrame(response.json())
    except requests.exceptions.ConnectionError:
        st.error("Connection Error: Could not fetch stock list.")
    return pd.DataFrame()

def api_update_stocks():
    try:
        response = requests.post(f"{API_BASE_URL}/update_stocks")
        return response.json()
    except requests.exceptions.ConnectionError:
        return {"success": False, "message": "Connection Error."}

# --- User Authentication UI ---
def show_login():
    st.title("Stock Portfolio Manager")
    st.subheader("Login")
    username = st.text_input("Username")
    password = st.text_input("Password", type='password')
    if st.button("Login"):
        result = api_login(username, password)
        if result and result.get("success"):
            st.session_state.logged_in = True
            st.session_state.username = result.get("username")
            st.session_state.user_id = result.get("userId")
            st.rerun()
        else:
            st.error(result.get("message", "Invalid username or password"))

# --- Main Application UI ---
def main_app():
    st.sidebar.title(f"Welcome, {st.session_state.username}")
    
    pages = {
        "My Portfolio": page_portfolio,
        "Stock Information": page_stock_info,
        "Update Stock Data": page_update_data
    }
    selection = st.sidebar.radio("Go to", list(pages.keys()))
    pages[selection]()
    
    if st.sidebar.button("Logout"):
        st.session_state.logged_in = False
        st.rerun()

def page_portfolio():
    st.title("My Portfolio")
    balance, portfolio_df = api_get_portfolio(st.session_state.user_id)
    
    st.metric(label="Available Funds", value=f"${balance:,.2f}")
    if portfolio_df.empty:
        st.info("Your portfolio is empty.")
    else:
        st.subheader("Current Holdings")
        st.dataframe(portfolio_df, use_container_width=True)

    st.subheader("Execute a Transaction")
    col1, col2 = st.columns(2)
    with col1:
        with st.form("buy_form", clear_on_submit=True):
            st.markdown("#### Buy Stocks")
            buy_symbol = st.text_input("Symbol").upper()
            buy_quantity = st.number_input("Quantity", min_value=1, step=1)
            buy_price = st.number_input("Price", min_value=0.01, format="%.2f")
            if st.form_submit_button("Execute Buy"):
                result = api_execute_transaction(st.session_state.user_id, buy_symbol, buy_quantity, buy_price, 'buy')
                if result.get("success"):
                    st.success("Buy order executed!")
                else:
                    st.error(f"Error: {result.get('message')}")

    with col2:
        with st.form("sell_form", clear_on_submit=True):
            st.markdown("#### Sell Stocks")
            sell_symbol = st.text_input("Symbol").upper()
            sell_quantity = st.number_input("Quantity", min_value=1, step=1)
            sell_price = st.number_input("Price", min_value=0.01, format="%.2f")
            if st.form_submit_button("Execute Sell"):
                result = api_execute_transaction(st.session_state.user_id, sell_symbol, sell_quantity, sell_price, 'sell')
                if result.get("success"):
                    st.success("Sell order executed!")
                else:
                    st.error(f"Error: {result.get('message')}")

def page_stock_info():
    st.title("Available Stock Information")
    stock_df = api_get_all_stocks()
    if stock_df.empty:
        st.warning("No stock data available.")
    else:
        st.dataframe(stock_df, use_container_width=True)

def page_update_data():
    st.title("Update Market Data")
    if st.button("Fetch Latest Stock Data"):
        with st.spinner("Requesting backend to fetch data..."):
            result = api_update_stocks()
            if result.get("success"):
                st.success("Backend successfully updated the stock database!")
            else:
                st.error(f"Error: {result.get('message')}")

# --- Main App Router ---
if not st.session_state.logged_in:
    show_login()
else:
    main_app()

