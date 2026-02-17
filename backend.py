from flask import Flask, request, jsonify, send_from_directory
import subprocess
import tempfile
import os
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

if os.name == 'nt':
    COMPILER_PATH = os.path.join(os.path.dirname(__file__), 'F4compiler_modular.exe')
else:
    COMPILER_PATH = os.path.join(os.path.dirname(__file__), 'f4compiler_modular')

@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    return send_from_directory('.', path)

@app.route('/compile', methods=['POST'])
def compile_code():
    code = request.json.get('code', '')
    if not code.strip():
        return jsonify({'error': 'No code provided.'}), 400
    with tempfile.NamedTemporaryFile(delete=False, suffix='.c', mode='w', dir=os.path.dirname(__file__)) as tmp:
        tmp.write(code)
        tmp_path = tmp.name
    try:
        result = subprocess.run([COMPILER_PATH, tmp_path], capture_output=True, text=True, timeout=10)
        output = result.stdout
        print("COMPILER OUTPUT:")
        print(output)
        # Improved section extraction
        def extract_section(name, text):
            import re
            # Match section header (with possible leading/trailing whitespace)
            pattern = re.compile(rf"^\s*{re.escape(name)}\s*$", re.MULTILINE)
            match = pattern.search(text)
            if not match:
                return ''
            start = match.end()
            # Find the next section header or end of text
            next_header = re.compile(r"^\s*===.*===\s*$", re.MULTILINE)
            next_match = next_header.search(text, start)
            end = next_match.start() if next_match else len(text)
            return text[start:end].strip()
        tokens = extract_section('=== Lexical Analysis (Tokenization) ===', output)
        ast = extract_section('=== Syntax Analysis (Parsing) ===', output)
        semantic = extract_section('=== Semantic Analysis ===', output)
        tac = extract_section('=== Intermediate Code Generation ===', output)
        asm = extract_section('=== Assembly Code Generation ===', output)
        print("TOKENS SECTION:")
        print(tokens)
        print("AST SECTION:")
        print(ast)
        print("SEMANTIC SECTION:")
        print(semantic)
        print("TAC SECTION:")
        print(tac)
        print("ASM SECTION:")
        print(asm)
        return jsonify({
            'tokens': tokens,
            'ast': ast,
            'semantic': semantic,
            'tac': tac,
            'asm': asm
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500
    finally:
        os.remove(tmp_path)

if __name__ == '__main__':
    app.run(debug=True, port=5000) 