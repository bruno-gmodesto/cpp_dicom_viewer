# DICOM Viewer

Visualizador profissional de imagens médicas (DICOM) desenvolvido com C++, Qt, VTK e DCMTK.

## 🚀 Funcionalidades

- **Visualização Universal**: Suporte a imagens Monocromáticas (8/12/16 bits) e Coloridas (RGB).
- **Processamento DICOM**:
  - Leitura robusta via DCMTK.
  - Suporte a diversas sintaxes de transferência (incluindo JPEG Lossless).
  - Extração automática de metadados.
- **Renderização Avançada**:
  - Pipeline de visualização baseada em VTK (Visualization Toolkit).
  - Manipulação de contraste/brilho em tempo real (Window Level / Window Width).
  - Correção automática de orientação (Flip Y).
- **Interface Gráfica**:
  - Interface moderna e responsiva (Qt 6).
  - Tema escuro (Dark Mode).
  - Painel lateral com metadados do paciente e exame.

## 🛠 Arquitetura do Projeto

O projeto segue princípios de arquitetura limpa e SOLID:

```
src/
├── ui/            → Camada de Apresentação (View)
│   ├── MainWindow.cpp      # Gerenciamento da janela principal e eventos UI
│   ├── mainwindow.ui       # Layout XML (Qt Designer)
│   └── styles/             # Gerenciamento de temas e estilos CSS
│
└── viewer/        → Núcleo de Visualização (Core)
    ├── DicomViewer.cpp     # Wrapper VTK + Facade de Carregamento
    └── DicomViewer.h       # Definições e Estruturas de Metadados
```

### Fluxo de Carregamento (Pipeline)
1. **DicomViewer** recebe o caminho do arquivo.
2. **DCMTK** carrega o dataset e extrai metadados (SRP).
3. O sistema detecta se a imagem é Colorida ou Monocromática.
4. **DicomImage** (para cor) ou extração raw (para P&B) converte os pixels.
5. Dados são transferidos para **vtkImageData** (utilizando templates DRY para cópia de memória).
6. **vtkImageViewer2** renderiza a imagem na widget Qt.

## 📦 Dependências

| Biblioteca | Versão Mínima | Função |
|------------|---------------|--------|
| **Qt**     | 6.2+          | Interface Gráfica (GUI) |
| **VTK**    | 9.0+          | Renderização de Imagens |
| **DCMTK**  | 3.6.7+        | Parser e Codecs DICOM |
| **CMake**  | 3.16+         | Sistema de Build |

## 🔨 Como Compilar

### Pré-requisitos
Certifique-se de ter as bibliotecas instaladas.
*   **macOS (Homebrew):** `brew install qt vtk dcmtk cmake`
*   **Ubuntu:** `sudo apt install qt6-base-dev libvtk9-dev libdcmtk-dev cmake`

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Executar

```bash
./dicom_viewer.app/Contents/MacOS/dicom_viewer  # macOS
./dicom_viewer                                  # Linux
```

## 📝 Padrões Adotados (Refatoração)

- **SRP (Single Responsibility Principle):** Separação clara entre extração de metadados, criação de imagem VTK e gerenciamento de UI.
- **DRY (Don't Repeat Yourself):** Utilização de templates C++ para lógica de cópia e espelhamento de buffers de pixel, evitando duplicação de código entre tipos de dados (Uint8, Uint16, RGB).
- **RAII:** Gerenciamento automático de recursos via `std::unique_ptr` e `vtkSmartPointer`.

## 📄 Licença

Este projeto é open-source. Sinta-se livre para estudar e modificar.