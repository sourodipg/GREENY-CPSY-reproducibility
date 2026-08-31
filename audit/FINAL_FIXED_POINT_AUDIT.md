# GREENY - Computational Psychiatry fixed-point submission audit

Date: 31 August 2026

## Scope
The audit compares the manuscript and reproducibility release against the current Computational Psychiatry submission guidance, then checks the latest numerical evidence and the bibliographic record. It stops at the first remaining items that cannot be resolved without author or repository metadata.

## Journal constraints checked

- Article format target: Tools and Methods (recommended); Research Article remains a fallback.
- Main-text word count: 7683+102 = 7785; CPsy limit: 8,000 (figure legends and references excluded).
- Abstract: 230 words; limit: 250.
- Display items: 8; limit: 8.
- Heading levels: level 1=6, level 2=18, level 3=0, level 4 paragraph-headings=0.
- Keywords: 6; limit: 6.
- Frozen-term occurrences in main manuscript: 0.
- Placeholder-like text hits: none detected.

## Structural result

- Citation keys match bibliography keys: True.
- Latest interaction rows: 560 (expected 560): True.
- Latest interaction outputs finite: True.
- Encounter-ledger equality: True.
- Latest interaction contrast reconstruction: True.
- BranchN latest evidence markers present: True.

## Hallucination audit

- References checked: 18/18 exact bibliographic records resolved to DOI or publisher/index record.
- No invented DOI or fabricated journal record was identified.
- The Solomon et al. (2000) statement that the mean number of major depressive episodes per year was 0.21 is supported by the PubMed abstract, so it is retained.
- The Wei et al. (2007) Study 6 anxiety/avoidance means and standard deviations (22.45/7.14 and 14.97/6.40) are confirmed in Table 7 of the article copy.
- The WAIS-IV standard-score convention (mean 100, SD 15) is supported by the APA description of the Wechsler scales and current Pearson WAIS-IV documentation.
- The paper uses literature sources as anchors or methodological context; model-design constants are explicitly labelled as design/inherited parameters rather than empirical constants.

## Remaining submission metadata

- Public repository DOI is not fabricated. The manuscript states that the DOI will be added when the archive is deposited.
- Funding provider/award identifier cannot be inferred safely from the computational archive and must be entered from the official funding record.
- Corresponding-author contact details are supplied through the journal submission metadata.

## Fixed-point decision

Within the information available to the computational record, no further scientific or editorial change is indicated by the checked requirements. The only remaining actions are external submission metadata: the actual public repository DOI and the official funding award identifier.