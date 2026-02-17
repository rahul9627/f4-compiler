async function compileCode() {
    const code = document.getElementById('codeInput').value;
    document.getElementById('tokensOutput').textContent = 'Compiling...';
    document.getElementById('astOutput').textContent = '';
    document.getElementById('semanticOutput').textContent = '';
    document.getElementById('tacOutput').textContent = '';
    document.getElementById('asmOutput').textContent = '';
    try {
        const response = await fetch('http://localhost:5000/compile', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ code })
        });
        const data = await response.json();
        if (data.error) {
            document.getElementById('tokensOutput').textContent = 'Error: ' + data.error;
            return;
        }
        document.getElementById('tokensOutput').textContent = data.tokens || '';
        document.getElementById('astOutput').textContent = data.ast || '';
        document.getElementById('semanticOutput').textContent = data.semantic || '';
        document.getElementById('tacOutput').textContent = data.tac || '';
        document.getElementById('asmOutput').textContent = data.asm || '';
    } catch (err) {
        document.getElementById('tokensOutput').textContent = 'Error: ' + err;
    }
} 