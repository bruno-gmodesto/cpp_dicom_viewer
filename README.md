# DICOM Viewer

Visualizador profissional de imagens médicas (DICOM) desenvolvido com C++, Qt, VTK e DCMTK.

## Funcionalidades

- **Visualização Universal**: Suporte a imagens Monocromáticas (8/12/16 bits) e Coloridas (RGB).
- **Processamento DICOM**:
  - Leitura via DCMTK.
  - Suporte a diversas sintaxes de transferência.
  - Extração automática de metadados.
- **Renderização Avançada**:
  - Pipeline de visualização baseada em VTK.
  - Manipulação de contraste/brilho em tempo real.
  - Correção automática de orientação.
- **Interface Gráfica**:
  - Interface moderna e responsiva.
  - Painel lateral com metadados do paciente e exame.

## 🛠 Arquitetura do Projeto

```
src/
├── ui/            → Camada de Apresentação
│   ├── MainWindow.cpp      # Gerenciamento da janela principal e eventos UI
│   ├── mainwindow.ui       # Layout XML
│   └── styles/             # Gerenciamento de temas e estilos CSS
│
└── viewer/        → Núcleo de Visualização
    ├── DicomViewer.cpp     # Wrapper VTK + Facade de  Carregamento
    └── DicomViewer.h       
    # Definições e Estruturas de Metadados
```

### Fluxo de Carregamento
1. **DicomViewer** recebe o caminho do arquivo.
2. **DCMTK** carrega o dataset e extrai metadados (SRP).
3. O sistema detecta se a imagem é Colorida ou Monocromática.
4. **DicomImage** (para cor) ou extração raw (para P&B) converte os pixels.
5. Dados são transferidos para **vtkImageData**.
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


## 📄 Licença

Este projeto é open-source. Sinta-se livre para estudar e modificar.