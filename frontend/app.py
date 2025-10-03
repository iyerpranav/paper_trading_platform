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
        payload = {"username": username, "password": password}
        response = requests.post(f"{API_BASE_URL}/login", json=payload)
        return response.json()
    except Exception as e:
        return {"success": False, "message": str(e)}

def api_signin(username, password, email):
    try:
        payload = {"username": username, "password": password, "email": email}
        response = requests.post(f"{API_BASE_URL}/signin", json=payload)
        return response.json()
    except Exception as e:
        return {"success": False, "message": str(e)}


def api_get_portfolio(user_id):
    try:
        response = requests.get(f"{API_BASE_URL}/portfolio/{user_id}")
        if response.status_code == 200:
            data = response.json()
            balance = data.get('fundBalance', 0.0)
            stocks = data.get('stocks', [])
            return balance, pd.DataFrame(stocks)
    except requests.exceptions.ConnectionError:
        st.error("Connection Error: Could not fetch portfolio. Is the C++ server running?")
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
        st.error("Connection Error: Could not fetch stock list. Is the C++ server running?")
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
    
    tab1, tab2 = st.tabs(["Login", "Sign Up"])

    # Login Form
    with tab1:
        st.subheader("Login")
        with st.form("login_form"):
            username = st.text_input("Username", key="login_username")
            password = st.text_input("Password", type='password', key="login_password")
            submitted = st.form_submit_button("Login")
            
            if submitted:
                if not username or not password:
                    st.error("Please enter both username and password")
                else:
                    result = api_login(username, password)
                    if result.get("success"):
                        st.session_state.logged_in = True
                        st.session_state.username = result.get("username")
                        st.session_state.user_id = result.get("userId")
                        st.rerun() # Use modern rerun
                    else:
                        st.error(result.get("message", "Invalid username or password"))

    # Sign Up Form
    with tab2:
        st.subheader("Sign Up")
        with st.form("signup_form"):
            username = st.text_input("Username", key="signup_username")
            password = st.text_input("Password", type='password', key="signup_password")
            email = st.text_input("Email", key="signup_email")
            submitted = st.form_submit_button("Sign Up")
            
            if submitted:
                if not username or not password or not email:
                    st.error("Please fill all fields")
                else:
                    result = api_signin(username, password, email)
                    if result.get("success"):
                        st.session_state.logged_in = True
                        st.session_state.username = result.get("username")
                        st.session_state.user_id = result.get("userId")
                        st.rerun() # Use modern rerun
                    else:
                        st.error(result.get("message", "Sign up failed. Username or email may already be in use."))


# --- Main Application UI ---
def main_app():
    st.sidebar.title(f"Welcome, {st.session_state.username}")
    
    pages = {
        "My Portfolio": page_portfolio,
        "Stock Information": page_stock_info,
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
    
    if st.button("Fetch Latest Stock Data"):
        with st.spinner("Requesting backend to fetch data..."):
            result = api_update_stocks()
            if result.get("success"):
                st.success("Backend successfully updated the stock database!")
            else:
                st.error(f"Error: {result.get('message')}")
    
    stock_df = api_get_all_stocks()
    if stock_df.empty:
        st.warning("No stock data available. Try fetching the latest data.")
    else:
        st.dataframe(stock_df, use_container_width=True)

# --- Main App Router ---
if not st.session_state.logged_in:
    show_login()
else:
    main_app()

